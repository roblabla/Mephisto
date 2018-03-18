#include "Ctu.h"

LogLevel g_LogLevel = Debug;

Ctu::Ctu() : cpu(this), svc(this), ipc(this), tm(this), mmiohandler(this), bridge(this), gdbStub(this), handleId(0xde00), heapsize(0x0) {
	handles[0xffff8001] = make_shared<Process>(this);
}

void Ctu::execProgram(gptr ep) {
	auto sp = 7 << 24;
	auto ss = 8 * 1024 * 1024;
	auto config = 6 << 24;

	LoaderConfigEntry entries[6];

	cpu.map(sp - ss, ss, UC_PROT_READ | UC_PROT_WRITE);
	mmiohandler.MMIOInitialize();
	cpu.setMmio(&mmiohandler);

	auto mainThread = tm.create(ep, sp);
	if (hbapi && loadType == "nro") {
		this->heapsize = 0x100000;

		// FIXME: correctly handle page mapping to avoid declaring multiple distinct pages
		for (guint offset = 0; offset < this->heapsize; offset += 4096) {
			cpu.map(0xaa0000000 + offset, 4096, UC_PROT_READ | UC_PROT_WRITE);
		}
		//cpu.map(0xaa0000000, this->heapsize, UC_PROT_READ | UC_PROT_WRITE);


		// Setup
		entries[0].key = MAIN_THREAD_HANDLE;
		entries[0].value[0] = mainThread->handle;
		entries[0].flags = 0;
		entries[1].key = APPLET_TYPE;
		entries[1].value[0] = APPLET_TYPE_APPLICATION;
		entries[1].flags = 0;
		entries[2].key = OVERRIDE_HEAP;
		entries[2].value[0] = 0xaa0000000;
		entries[2].value[1] = this->heapsize;
		entries[2].flags = 0;
		entries[3].key = PROCESS_HANDLE;
		entries[3].value[0] = 0xffff8001;
		entries[3].flags = 0;

		cpu.map(0xbb0000000, 0x20000, UC_PROT_READ | UC_PROT_WRITE);
		entries[4].key = LOG;
		entries[4].value[0] = 0xbb0000000;
		entries[4].value[1] = 0x20000;
		entries[4].flags = 0;

		entries[5].key = END_OF_LIST;

		cpu.writemem(config, &entries, 5 * sizeof(LoaderConfigEntry));

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
