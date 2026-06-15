#pragma once
#include "Type.h"
#include <string>

class Player
{
public:
	void Init(int64 sessionId, WCHAR* ip);

public:
	int64 _sessionId = 0; // sessionid,  character id
	SRWLOCK _lock;
	int64 accountNo = 0;
	uint32 _lastRecvTime = 0;

	WCHAR id[20];
	WCHAR nickName[20];
	uint16 _index = 0;
	bool _bLogined = false;

	std::string _sessionKey;
	WCHAR _ip[INET_ADDRSTRLEN] = L"";
};


