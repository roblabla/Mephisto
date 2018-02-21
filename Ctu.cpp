#include "Ctu.h"

LogLevel g_LogLevel = Debug;

Ctu::Ctu() : cpu(this), svc(this), ipc(this), tm(this), mmiohandler(this), bridge(this), gdbStub(this), handleId(0xde00), heapsize(0x0) {
	handles[0xffff8001] = make_shared<Process>(this);
}

void Ctu::execProgram(gptr ep) {
	auto sp = 7 << 24;
	auto ss = 8 * 1024 * 1024;
	auto config = 6 << 24;

	LoaderConfigEntry entries[3];

	cpu.map(sp - ss, ss, UC_PROT_READ | UC_PROT_WRITE);
	mmiohandler.MMIOInitialize();
	cpu.setMmio(&mmiohandler);

	auto mainThread = tm.create(ep, sp);
	if (hbapi && loadType == "nro") {
		// Setup
		entries[0].key = MAIN_THREAD_HANDLE;
		entries[0].value[0] = mainThread->handle;
		entries[1].key = APPLET_TYPE;
		entries[1].value[0] = APPLET_TYPE_APPLICATION;
		entries[2].key = END_OF_LIST;

		cpu.writemem(config, &entries, 3 * sizeof(LoaderConfigEntry));

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
