#pragma once

#include "Ctu.h"
#include <sys/time.h>

struct Timeout {
	Timeout(struct timespec time, function<void()> cb) : time(time), cb(cb) {}
	struct timespec time;
	function<void()> cb;
};

class TimeoutManager {
public:
	TimeoutManager(Ctu *_ctu);

	void start();
	int create(struct timespec time, function<void()> cb);
	int create(guint timeout, function<void()> cb);
	void done(int i);
	bool is_empty();
private:
	void run();
	int curidx;
	unordered_map<int, Timeout> timeouts;
	Ctu *ctu;
};
