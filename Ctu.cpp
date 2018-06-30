#include "Ctu.h"
#include<arpa/inet.h>

LogLevel g_LogLevel = Debug;

Ctu::Ctu() : cpu(this), svc(this), ipc(this), tm(this), mmiohandler(this), bridge(this), gdbStub(this), handleId(0xde00), heapsize(0x0) {
	handles[0xffff8001] = make_shared<Process>(this);
}

void Ctu::execProgram(gptr ep) {
	auto sp = 7 << 24;
	auto ss = 0xa0000;
	auto config = 6 << 24;

	LoaderConfigEntry entries[8];

	cpu.map(sp - ss, ss, UC_PROT_READ | UC_PROT_WRITE);
	mmiohandler.MMIOInitialize();
	cpu.setMmio(&mmiohandler);

	auto mainThread = tm.create(ep, sp);
	if (hbapi && loadType == "nro") {
		this->heapsize = 0x6400000;

		// FIXME: correctly handle page mapping to avoid declaring multiple distinct pages
		//for (guint offset = 0; offset < this->heapsize; offset += 4096) {
		//	cpu.map(0xaa0000000 + offset, 4096, UC_PROT_READ | UC_PROT_WRITE);
		//}
		cpu.map(0xaa0000000, this->heapsize, UC_PROT_READ | UC_PROT_WRITE);


		// Setup
		int i = 0;

		entries[i].key = MAIN_THREAD_HANDLE;
		entries[i].value[0] = mainThread->handle;
		entries[i++].flags = 0;
		entries[i].key = APPLET_TYPE;
		entries[i].value[0] = APPLET_TYPE_APPLICATION;
		entries[i++].flags = 0;
		entries[i].key = OVERRIDE_HEAP;
		entries[i].value[0] = 0xaa0000000;
		entries[i].value[1] = this->heapsize;
		entries[i++].flags = 0;
		entries[i].key = PROCESS_HANDLE;
		entries[i].value[0] = 0xffff8001;
		entries[i++].flags = 0;

		cpu.map(0xbb0000000, 0x20000, UC_PROT_READ | UC_PROT_WRITE);
		entries[i].key = LOG;
		entries[i].value[0] = 0xbb0000000;
		entries[i].value[1] = 0x20000;
		entries[i++].flags = 0;

		auto bsd_u = make_shared<nn::socket::sf::IClient>(this);
		auto handle = newHandle(bsd_u);
		entries[i].key = OVERRIDE_SERVICE;
		entries[i].value[0] = (uint64_t)'b' | (uint64_t)'s' << 8 | (uint64_t)'d' << 16 | (uint64_t)':' << 24 | (uint64_t)'u' << 32;
		printf("Value is %.*s\n", 8, (char*)&entries[5].value[0]);
		entries[i].value[1] = handle;
		entries[i++].flags = 0;
;

		std::cout << "Doing socket stuff" << std::endl;

		int32_t socket;
		int32_t ret;
		uint32_t bsd_errno;
		struct sockaddr_in sockaddr;
		sockaddr.sin_family = AF_INET << 8;
		sockaddr.sin_port = htons(2991);
		inet_pton(AF_INET, "127.0.0.1", &sockaddr.sin_addr);
		bsd_u->Socket(AF_INET, SOCK_STREAM, 0, socket, bsd_errno);
		if (bsd_errno == 0) {
			bsd_u->Connect(socket, reinterpret_cast<struct sockaddr*>(&sockaddr), sizeof(sockaddr), ret, bsd_errno);
			if (bsd_errno == 0) {
				std::cout << "Socket is " << socket << std::endl;
				entries[i].key = STDIO_SOCKETS;
				entries[i].value[0] = (uint64_t)socket | (uint64_t)socket << 32;
				entries[i].value[1] = socket;
				i++;
			} else {
				perror("connect");
			}
		} else {
			perror("socket");
		}

		entries[i].key = END_OF_LIST;

		cpu.writemem(config, &entries, (i + 1) * sizeof(LoaderConfigEntry));

		mainThread->regs.X0 = config;
		mainThread->regs.X1 = 0xFFFFFFFFFFFFFFFF;
	}
	else {
		mainThread->regs.X1 = mainThread->handle;
	}
	mainThread->resume();

	tm.start();
}

ghandle Ctu::duplicateHandle(KObject *ptr) {
	for(auto elem : handles)
		if(elem.second.get() == ptr)
			return newHandle(elem.second);
	return 0;
}

void Ctu::deleteHandle(ghandle handle) {
	if(handles.find(handle) != handles.end()) {
		auto hnd = getHandle<KObject>(handle);
		handles.erase(handle);
		hnd->close();
	}
}
