#pragma once

#include "Ctu.h"

class Waitable : public KObject {
public:
	Waitable();

	void acquire();
	void release();

	int wait(function<int()> cb);
	virtual int wait(function<int(bool)> cb);
	void unwait(int id);

	void signal(bool one=false);
	void cancel();

	virtual void close() override {}

protected:
	virtual bool presignalable() { return true; }

private:
	recursive_mutex lock;
	unordered_map<int, function<int(bool)>> waiters;
	int idx;
	bool presignaled, canceled;
};

class InstantWaitable : public Waitable {
public:
	virtual int wait(function<int(bool)> cb) {
		int res = Waitable::wait(cb);
		signal(true);
		return res;
	}
};

class Semaphore : public Waitable {
public:
	Semaphore(Guest<uint32_t> _vptr);

	void increment();
	void decrement();

	uint32_t value();

protected:
	bool presignalable() override { return false; }

private:
	Guest<uint32_t> vptr;
};

class Mutex : public Waitable {
public:
	Mutex(Guest<uint32_t> _vptr);

	uint32_t value();
	void value(uint32_t val);
	
	ghandle owner();
	void owner(ghandle val);

	bool hasWaiters();
	void hasWaiters(bool val);

	void guestRelease();

protected:
	bool presignalable() override { return false; }

private:
	Guest<uint32_t> vptr;
};
