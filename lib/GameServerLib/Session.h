#pragma once
#include "RingBuffer.h"
#include "type.h"
#include "LockFreeQueue.h"
#include <queue>


//전방선언
class CPacket;

struct IoFlag {
	long _releaseFlag;
	long _ioCount;

};

struct Session
{
	Session()
	{
		_ioFlag._releaseFlag = 0;
		_ioFlag._ioCount = 1;
	}

	~Session()
	{
		
	}

	int64 _sessionId;
	// send용 overlapp
	OVERLAPPED _sendOverlapped;
	// recv용 overlapp
	OVERLAPPED _recvOverlapped;
	// 소켓
	SOCKET _socket = INVALID_SOCKET;
	// 수신 큐
	RingBuffer _recvQueue;
	//송신 큐
	LockFreeQueue<CPacket*> _sendQueue;
	std::queue<CPacket*> _sendedQueue;

	// ip
	WCHAR _ip[INET_ADDRSTRLEN] = L"";
	USHORT _port = 0;
	// 송신중인지
	bool _isSending = false;
	// 연결 해제용 io count
	IoFlag _ioFlag;
	bool _disconnectRequested = false;
	
	//GameThread
	class GameThread* _gameThread;
	LockFreeQueue<CPacket*> _packetQueue;
	uint16 _gameThreadArrIndex;
	SRWLOCK _lock;

	void Init(int64 sessionId, SOCKET socket, SOCKADDR_IN* clientAddr);
	void Init(int64 sessionId, SOCKET socket);

	void ClearSendedQueue();
	void ClearSendQueue();
	void ClearPacketQueue();

	//long _bProcessingAsyncJobNum = 0;
};



