#include "Ctu.h"

uint32_t Pm::Shell::LaunchTitle(IN uint64_t _0, IN nn::ApplicationId tid) {
	LOG_DEBUG(Pm::Shell, "Attempted to launch title " ADDRFMT, tid);
	return 0;
}

uint32_t Pm::Shell::GetProcessEventWaiter(OUT shared_ptr<KObject>& _0) {
	LOG_DEBUG(IpcStubs, "Stub implementation for Pm::Shell::GetProcessEventWaiter");
	_0 = make_shared<Waitable>();
	return 0;
}
