#pragma once
#include "Type.h"
#include "Session.h"
#include <stack>
#include "LockFreeStack.h"
#include "GameThread.h"
#include <vector>
#include "Data.h"	
#define MAX_SESSION_NUM 20000
#define CHECK_RELEASE 999999

class CPacket;

class CNetServer
{
	friend class GameThread;
public:
	CNetServer();

	virtual ~CNetServer();

	// nagle option, sendzerocopy
	bool Start(uint16 port, uint32 concurrentThreadNum, uint32 workerThreadNum, int nagle = 1, int sendZeroCopy = 0); // open ip, port, worker thread num, nagle option, maximum client num;
	void Stop();
	void Disconnect(int64 sessionId);
	void SendPacket(int64 sessionId, CPacket* packet);
	void SendPackets(int64 sessionId, std::vector<CPacket*>& packet);

	/// <summary>
	/// return false to reject client accept
	/// else return true
	/// </summary>
	/// <returns> </returns>
	virtual bool OnConnectionRequest() = 0;
	virtual void OnError(int errorCode, WCHAR* errorMessage) = 0;


private:
	// iocp worker thread
	static unsigned __stdcall  WorkerThreadStatic(void* param)
	{
		CNetServer* thisClass = (CNetServer*)param;
		return thisClass->WorkerThread();
	}

	unsigned __stdcall WorkerThread();
	
	// accept thread
	static unsigned __stdcall AcceptThreadStatic(void* param)
	{
		CNetServer* thisClass = (CNetServer*)param;
		return thisClass->AcceptThread();
	}
	unsigned __stdcall AcceptThread();

	// monitoring thread
	static unsigned __stdcall MonitorThreadStatic(void* param)
	{
		CNetServer* thisClass = (CNetServer*)param;
		return thisClass->MonitorThread();
	}
	unsigned __stdcall MonitorThread();
	
	// release thread
	static unsigned __stdcall ReleaseThreadStatic(void* param)
	{
		CNetServer* thisClass = (CNetServer*)param;
		return thisClass->ReleaseThread();
	}
	unsigned __stdcall ReleaseThread();


	bool SendPost(Session* s);
	bool RecvPost(Session* s);
	bool AcceptProcess(SOCKET socket, SOCKADDR_IN* clientAddr);
	void Disconnect(Session* session);

public:
	bool isNetworkStop()
	{
		return _bStopNetwork;
	}

private:
	Session _sessionArr[MAX_SESSION_NUM];

private:
	HANDLE _iocpHandle = NULL;
	bool _bStopNetwork = false;
	int _threadNumber = 0;
	HANDLE _threadList[100];
	SOCKET _listenSocket = INVALID_SOCKET;
	uint16 _serverPort = 0;

	
public: //logging
#define TPS_ARR_NUM 5


	int64 GetAcceptTPS()
	{
		static int acceptIndex = 0;
		return _monitoredAcceptTPS[acceptIndex++ % TPS_ARR_NUM];
	}

	int64 GetRecvMessageTPS()
	{
		static int recvIndex = 0;
		return _monitoredRecvTPS[recvIndex++ % TPS_ARR_NUM];
	}

	int64 GetSendMessageTPS()
	{
		static int sendIndex = 0;
		return _monitoredSendTPS[sendIndex++ % TPS_ARR_NUM];
	}

	int32 GetNetworkSendByteTps()
	{
		static int sendIndex = 0;
		return _monitoredNetworkByteSendTPS[sendIndex++ % TPS_ARR_NUM];
	}

	int32 GetNetworkRecvByteTps()
	{
		static int sendIndex = 0;
		return _monitoredNetworkByteRecvTPS[sendIndex++ % TPS_ARR_NUM];
	}


	int64 GetTotalAcceptValue()
	{
		return _totalAccept;
	}

	int64 GetTotalDisconnect()
	{
		return _totalDisconnet;
	}

	int64 GetRecvConnResetTotal()
	{
		return _recvConnResetTotal;
	}


	int64 GetSessionNum()
	{
		return _sessionNum;
	}

	int64 GetReleaseFlagDisconnectNum()
	{
		return _releaseFlageDisconnect;
	}

	int64 GetDisconnectSessionNumBySendQueueFull()
	{
		return _disconnectSessionNumBySendQueueFull;
	}


protected:
	int64 GetSessionId(int64 sessionId);

private: // 肺彪
	int64 _monitoredSendTPS[TPS_ARR_NUM] = { 0, };
	int64 _monitoredRecvTPS[TPS_ARR_NUM] = { 0, };
	int64 _monitoredAcceptTPS[TPS_ARR_NUM] = { 0, };
	int32 _monitoredNetworkByteSendTPS[TPS_ARR_NUM] = { 0, };
	int32 _monitoredNetworkByteRecvTPS[TPS_ARR_NUM] = { 0, };

	int64 _processSendPacket = 0;
	int64 _processRecvPacket = 0;
	int64 _acceptPacket = 0;
	long _networkSend = 0;
	long _networkRecv = 0;


	int64 _totalDisconnet = 0;
	int64 _totalAccept = 0;
	int64 _recvConnResetTotal = 0;
	int64 _sessionNum;
	int64 _releaseFlageDisconnect;
	int64 _disconnectSessionNumBySendQueueFull = 0;

private: // 技记
	Session* CreateSession(SOCKET socket, SOCKADDR_IN* clientAddr);
	bool TryReleaseSession(Session* session);
	bool TryReleaseSession(Session* session, int line);
	bool ReleaseSession(Session* session);
	uint16 GetSessionIndex(int64 sessionId);
	void InitSessionArr();
	void ClearSesssionArr();

private: // 技记 硅凯
	LockFreeStack<uint16> _emptySessionIndex;
	SRWLOCK _sessionArrLock;

private: // Release Thread
	LockFreeQueue<Session*> _releaseSessionQueue;
	HANDLE hReleaseEvent = NULL;

private:
	void IncIoCount(Session* session);
	bool DecIoCount(Session* session);
	Session* GetSession(int64 sessionId);
	void PutBackSession(Session* session);
	void ReqWSASend(Session* session);


private: 
	//nagle option default true
	int _nagle = 1;
	//send zero copy option default false;
	int _sendZeroCopy = 0;


protected:
	// TODO: CreateGameThread
	void RegisterGameThread(GameThread* gameThread);
	void RegisterDefaultGameThread(GameThread* gameThread);

	GameThread* _defaultGameThread;

private:
	uint16 _packetKey = Data::serverPacketKey;
};

