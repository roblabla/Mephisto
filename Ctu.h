#pragma once

#include <netinet/in.h>
#include <stdint.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <algorithm>
#include <array>
#include <cassert>
#include <climits>
#include <forward_list>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <numeric>
#include <unordered_map>
#include <queue>
#include <string>
#include <thread>
#include <tuple>
#include <type_traits>
#include <vector>
#include <sstream>
#include "hb-api.h"
using namespace std;

typedef __int128_t int128_t;
typedef __uint128_t uint128_t;
typedef float float32_t;
typedef double float64_t;
typedef uint64_t gptr;
typedef uint64_t guint;
typedef uint8_t guchar;
typedef uint16_t gushort;
typedef uint32_t ghandle;
typedef uint64_t gpid;
const gptr TERMADDR = 1ULL << 61;

#define FOURCC(a, b, c, d) (((d) << 24) | ((c) << 16) | ((b) << 8) | (a))

#ifdef __APPLE__
	#define ADDRFMT "%016llx"
	#define LONGFMT "%llx"
#else
	#define ADDRFMT "%016lx"
	#define LONGFMT "%lx"
#endif

enum LogLevel {
	None = 0,
	Error = 1,
	Warn = 2,
	Debug = 3,
	Info = 4
};

extern LogLevel g_LogLevel;

#define LOG_ERROR(module, msg, ...) do { \
	if(g_LogLevel >= Error) { \
		fprintf(stderr, "[" #module "] ERROR: " msg "\n", ##__VA_ARGS__); \
		cerr << endl; \
	} \
	exit(1); \
} while(0)
#define LOG_INFO(module, msg, ...) do { \
	if(g_LogLevel >= Info) { \
		printf("[" #module "] INFO: " msg, ##__VA_ARGS__); \
		cout << endl; \
	} \
} while(0)
#define LOG_DEBUG(module, msg, ...) do { \
	if(g_LogLevel >= Debug) { \
		printf("[" #module "] DEBUG: " msg, ##__VA_ARGS__); \
		cout << endl; \
	} \
} while(0)

class Ctu;
class LogMessage;

enum class BreakpointType {
	None,
	Execute,
	Read,
	Write,
	Access
};

typedef struct {
	float64_t low;
	float64_t high;
} float128;

typedef struct {
	guint SP, PC, NZCV;
	union {
		struct {
			guint gprs[31];
			float128 fprs[32];
		};
		struct {
			guint  X0,  X1,  X2,  X3, 
			       X4,  X5,  X6,  X7, 
			       X8,  X9, X10, X11, 
			      X12, X13, X14, X15, 
			      X16, X17, X18, X19, 
			      X20, X21, X22, X23, 
			      X24, X25, X26, X27, 
			      X28, X29, X30;
			float128   Q0,  Q1,  Q2,  Q3,
			           Q4,  Q5,  Q6,  Q7, 
			           Q8,  Q9, Q10, Q11, 
			          Q12, Q13, Q14, Q15, 
			          Q16, Q17, Q18, Q19, 
			          Q20, Q21, Q22, Q23, 
			          Q24, Q25, Q26, Q27, 
			          Q28, Q29, Q30, Q31;
		};
	};
} ThreadRegisters;

#include "optionparser.h"
#include "Lisparser.h"
#include "KObject.h"
#include "Mmio.h"
#include "Cpu.h"
#include "Sync.h"
#include "ThreadManager.h"
#include "Svc.h"
#include "Ipc.h"
#include "Nxo.h"
#include "IpcBridge.h"
#include "GdbStub.h"

template<unsigned long N>
void hexdump(shared_ptr<array<uint8_t, N>> buf, unsigned long count=N) {
	if(g_LogLevel < Debug)
		return;

	for(auto i = 0; i < count; i += 16) {
		printf("%04x | ", i);
		for(auto j = 0; j < 16; ++j) {
			printf("%02x ", buf->data()[i + j]);
			if(j == 7)
				printf(" ");
		}
		printf("| ");
		for(auto j = 0; j < 16; ++j) {
			auto val = buf->data()[i + j];
			if(isprint(val))
				printf("%c", val);
			else
				printf(".");
			if(j == 7)
				printf(" ");
		}
		printf("\n");
	}
	printf("%04x\n", (int) count);
}

class Ctu {
public:
	Ctu();
	void execProgram(gptr ep);

	template<typename T>
	ghandle newHandle(shared_ptr<T> obj) {
		static_assert(std::is_base_of<KObject, T>::value, "T must derive from KObject");
		auto hnd = handleId++;
		LOG_DEBUG(Ctu, "Creating handle %x", hnd);
		handles[hnd] = dynamic_pointer_cast<KObject>(obj);
		return hnd;
	}
	template<typename T>
	shared_ptr<T> getHandle(ghandle handle) {
		static_assert(std::is_base_of<KObject, T>::value, "T must derive from KObject");
		if(handles.find(handle) == handles.end())
			LOG_ERROR(Ctu, "Could not find handle with ID 0x%x !", handle);
		auto obj = handles[handle];
		auto faux = dynamic_pointer_cast<FauxHandle>(obj);
		if(faux != nullptr)
			LOG_ERROR(Ctu, "Accessing faux handle! 0x%x", faux->val);
		auto temp = dynamic_pointer_cast<T>(obj);
		if(temp == nullptr)
			LOG_ERROR(Ctu, "Got null pointer after cast. Before: 0x%p", (void *) obj.get());
		return temp;
	}

	void *getMapping(gptr addr) {
		void *data = mappings[addr];
		if (data == nullptr)
			LOG_ERROR(Ctu, "Could not find mapping with addr" LONGFMT "!", addr);
		return data;
	}

	bool newMapping(gptr addr, void *data) {
		void *old_data = mappings[addr];
		if (old_data == nullptr) {
			mappings[addr] = data;
			return true;
		}
		return false;
	}

	void *deleteMapping(gptr addr) {
		void *old_data = mappings[addr];
		if (old_data != nullptr) {
			mappings[addr] = nullptr;
			for (auto mapping : mappings) {
				if (mapping.second == old_data)
					return old_data;
			}
			free(old_data);
		}
		return nullptr;
	}

	bool hasHandle(ghandle handle) {
		return handles.find(handle) != handles.end();
	}

	ghandle duplicateHandle(KObject *ptr);
	void deleteHandle(ghandle handle);

	Cpu cpu;
	Mmio mmiohandler;
	Svc svc;
	Ipc ipc;
	ThreadManager tm;
	IpcBridge bridge;
	GdbStub gdbStub;

	guint heapsize;
	gptr loadbase, loadsize;
	string loadType;

	bool socketsEnabled;
	bool hbapi;

private:
	ghandle handleId;
	unordered_map<ghandle, shared_ptr<KObject>> handles;
	unordered_map<gptr, void*> mappings;
};

#include "IpcStubs.h"
