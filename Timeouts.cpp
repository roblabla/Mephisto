#include "Ctu.h"

TimeoutManager::TimeoutManager(Ctu *_ctu) : ctu(_ctu), timeouts(), curidx(1) {
}


void TimeoutManager::start() {
	LOG_INFO(TimeoutManager, "Starting");

	auto thread = ctu->tm.createNative([this] { run(); });
	thread->resume();
}

void TimeoutManager::run() {
	struct timespec curtime;
	clock_gettime(CLOCK_MONOTONIC, &curtime);
	for (auto it = timeouts.begin(); it != timeouts.end(); ) {
		if ((*it).second.time.tv_sec <= curtime.tv_sec && (*it).second.time.tv_nsec <= curtime.tv_nsec) {
			LOG_INFO(TimeoutManager, "Timeout %d reached!", (*it).first);
			(*it).second.cb();
			it = timeouts.erase(it);
		} else {
			it++;
		}
	}
}

bool TimeoutManager::is_empty() {
	return timeouts.empty();
}

int TimeoutManager::create(struct timespec time, function<void()> cb) {
	auto idx = curidx++;
	LOG_INFO(TimeoutManager, "Creating timeout %d for time %lu !", idx, time.tv_sec);
	timeouts.insert({idx, Timeout(time, cb)});
	return idx;
}

int TimeoutManager::create(guint timeout, function<void()> cb) {
	struct timespec curtime;

	// Avoid creating a bunch of useless timeouts.
	// TODO: Check correctness
	if (timeout == (guint)-1)
		return 0;

	clock_gettime(CLOCK_MONOTONIC, &curtime);
	curtime.tv_sec += timeout / (1000 * 1000 * 1000);
	curtime.tv_nsec += timeout % (1000 * 1000 * 1000);
	if (curtime.tv_nsec >= 1000 * 1000 * 1000) {
		curtime.tv_nsec -= 1000 * 1000 * 1000;
		curtime.tv_sec++;
	}
	return create(curtime, cb);
}

void TimeoutManager::done(int i) {
	if (i == 0)
		return;

	LOG_INFO(TimeoutManager, "Erasing timeout %d", i);
	timeouts.erase(i);
}
