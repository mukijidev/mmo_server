#pragma once
#include "Type.h"

class CPacket;

struct Job
{
	Job()
	{

	}
	
	Job(bool bSysJob, uint16 jobType, int64 sessionId, CPacket* packet)
	{
		_bSysJob = bSysJob;
		_jobType = jobType;
		_sessionId = sessionId;
		_packet = packet;
	}

	void Init(bool bSysJob, uint16 jobType, int64 sessionId, CPacket* packet)
	{
		_bSysJob = bSysJob;
		_jobType = jobType;
		_sessionId = sessionId;
		_packet = packet;
	}

	bool _bSysJob = false;
	uint16 _jobType = 0;
	int64 _sessionId = 0;
	CPacket* _packet = nullptr;
};

