#pragma once
#include "Session.h"
#include <unordered_map>
#include <algorithm>
#include "Type.h"


class Player
{
	friend class ChattingServer;
private:
	
	void Init(int64 sessionId);

private:
	int64 _sessionId = 0; // sessionid,  character id
	int64 accountNo = 0;
	uint32 _lastRecvTime = 0;
	bool _bLogined = false;
	TCHAR NickName[20];
};

