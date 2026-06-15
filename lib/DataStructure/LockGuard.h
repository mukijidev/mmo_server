#pragma once
#include "type.h"

class ReadLock
{
public:
	ReadLock(SRWLOCK* srwLock)
	{
		_srwLock = srwLock;
		AcquireSRWLockShared(_srwLock);
	}

	~ReadLock()
	{
		ReleaseSRWLockShared(_srwLock);
	}

private:
	SRWLOCK* _srwLock = nullptr;
};

class WriteLock
{
public:
	WriteLock(SRWLOCK* srwLock)
	{
		_srwLock = srwLock;
		AcquireSRWLockExclusive(_srwLock);
	}

	~WriteLock()
	{
		ReleaseSRWLockExclusive(_srwLock);
	}

private:

	SRWLOCK* _srwLock = nullptr;
};

class CSLock
{
public:
	CSLock(CRITICAL_SECTION* cs)
	{
		_cs = cs;
		EnterCriticalSection(_cs);
	}

	~CSLock()
	{
		LeaveCriticalSection(_cs);
	}

private:
	CRITICAL_SECTION* _cs;
};

class LockGuard
{
};
