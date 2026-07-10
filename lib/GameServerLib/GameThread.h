#pragma once

#include "LockFreeQueue.h"
#include "Session.h"
#include "LockFreeStack.h"
#include <vector>
#include <map>
#include <functional>
#include <atomic>
#include <memory>


#define TPS_ARR_NUM 5
#define GAMETHREAD_MAX_SESSION_NUM 6000

#define ASYNC_JOB_THREAD_INDEX_ONE 0
#define ASYNC_JOB_THREAD_INDEX_TWO 1

#define ASYNC_JOG_THRAED_NUM 2



struct AsyncJob
{
	int64 objectId;
	std::function<void()> job;
	uint16 jobType;
	std::shared_ptr<void> outResult; //out
};

struct AsyncJobResult
{
	int64 objectId;
	uint16 jobType;
	std::shared_ptr<void> result;
};

struct MoveThreadInfo
{
	int64 sessionId;
	int threadFrom;
	int threadTo;
	void* moveInfo;
};

struct EnterSessionInfo
{
	int64 sessionId;
	void* ptr;
};

struct LeaveSessionInfo
{
	int64 sessionId;
	bool disconnect;
};

class CNetServer;
// 이것을 라이브러리 차원에서 어떻게하지
class CPacket;

extern std::map<int, class GameThread*> _gameThreadInfoMap;

class GameThread
{
	friend class CNetServer;

public:
	void Start();
	GameThread(int threadId, const int msPerFrame);

	int _msPerFrame;
	int _originalMsPerFrame;

	int64 GetFps()
	{
		static int sendIndex = 0;
		return _packetTps[sendIndex++ % TPS_ARR_NUM];
	}

	int64 GetJPSQueueSize()
	{
		return _asyncJobQueue[0].Size();
	}

	int GetPlayerJpsQueueSize() { return (int)_asyncJobQueue[ASYNC_JOB_THREAD_INDEX_ONE].Size(); }
	int GetMonsterJpsQueueSize() { return (int)_asyncJobQueue[ASYNC_JOB_THREAD_INDEX_TWO].Size(); }

	int64 GetFrameAvgMs() { static int i = 0; return _frameAvgArr[i++ % TPS_ARR_NUM]; }
	int64 GetFrameMaxMs() { static int i = 0; return _frameMaxArr[i++ % TPS_ARR_NUM]; }

	int GetPlayerJpsTps() { static int i = 0; return (int)_playerJpsTpsArr[i++ % TPS_ARR_NUM]; }
	int GetMonsterJpsTps() { static int i = 0; return (int)_monsterJpsTpsArr[i++ % TPS_ARR_NUM]; }
	int GetDbWriteTps() { static int i = 0; return (int)_dbWriteTpsArr[i++ % TPS_ARR_NUM]; }

	int64 GetJPSFps()
	{
		static int sendIndex = 0;
		return _jpsTpsArr[sendIndex++ % TPS_ARR_NUM];
	}

	// 이 game thread 처리
private:

	virtual int64 GetPlayerSize() = 0;
	virtual void HandleRecvPacket(int64 sessionId, CPacket* packet) = 0;
	virtual void HandleAsyncJobFinish(int64 objectId, uint16 jobType, std::shared_ptr<void> result) = 0;

	static unsigned __stdcall UpdateThreadStatic(void* param)
	{
		GameThread* thisClass = (GameThread*)param;
		return thisClass->UpdateThread();
	}
	unsigned __stdcall UpdateThread();
	HANDLE _hUpdateThread;

	static unsigned int __stdcall MonitorThreadStatic(void* arg)
	{
		GameThread* pThis = (GameThread*)arg;
		return pThis->MonitorThread();
	}
	unsigned int __stdcall MonitorThread();
	HANDLE _hMonitorThread;

	static unsigned int __stdcall AsyncJobThreadStatic(void* arg)
	{
		GameThread* pThis = (GameThread*)arg;
		return pThis->AsyncJobThread();
	}
	unsigned int __stdcall AsyncJobThread();

	HANDLE _hAsyncJobThread[ASYNC_JOG_THRAED_NUM];
	HANDLE _hAsyncJobThreadEvent[ASYNC_JOG_THRAED_NUM];
	/*HANDLE _hAsyncJobThread;
	HANDLE _hAsyncJobThreadEvent;*/

	int64 _packetTps[TPS_ARR_NUM] = { 0, };
	int64 _updateTps = 0;
	int64 _jpsTpsArr[TPS_ARR_NUM] = { 0, };
	int64 _jpsTps = 0;

	int64 _frameMsAccum = 0;
	int   _frameTickCount = 0;
	int   _frameMsMax = 0;
	int64 _frameAvgArr[TPS_ARR_NUM] = { 0, };
	int64 _frameMaxArr[TPS_ARR_NUM] = { 0, };

protected:
	int64 _playerJpsTps = 0, _monsterJpsTps = 0, _dbWriteTps = 0;
	int64 _playerJpsTpsArr[TPS_ARR_NUM] = { 0, };
	int64 _monsterJpsTpsArr[TPS_ARR_NUM] = { 0, };
	int64 _dbWriteTpsArr[TPS_ARR_NUM] = { 0, };

private:
	//TODO: fram 처리
	void NetworkRun();
	virtual void GameRun(float deltaTime) = 0;
	void ProcessAsyncResults();

private:
	bool EnterSession(int64 sessionId, void* ptr);
	void LeaveSession(int64 sessionId, bool disconnect);
	


public:
	//TODO: 게임쓰레드 이동 처리
	virtual void OnLeaveThread(int64 sessionId, bool disconnect) = 0;
	void SendPackets_Unicast(int64 sessionId, std::vector<CPacket*>& packets);
	void SendPacket_Unicast(int64 sessionId, CPacket* packet);
	bool MoveGameThread(int gameThreadId, int64 sessionId, void* ptr);


public:
	virtual void OnEnterThread(int64 sessionId, void* ptr) = 0;
	
public:
	void SetGameServer(CNetServer* server)
	{
		_netServer = server;
	}

private:
	CNetServer* _netServer = nullptr;

private: 
	LockFreeStack<uint16> _emptySessionIndex;
	std::vector<int64> _sessionArr;
	LockFreeQueue<EnterSessionInfo> _enterQueue;
	LockFreeQueue<LeaveSessionInfo> _leaveQueue;

protected:
	void ProcessEnter();
	void ProcessLeave();
	//TODO: lambda 넣을 수 있게
	bool RequestAsyncJob(int64 objectId, std::function<void()> job, uint16 queueIndex, uint16 jobType, std::shared_ptr<void> outResult);
	

public:
	void Stop();
	bool CheckDead();
	void Kill();
	int64 GetSessionNum();
	//std::vector<int64> GetSessions();


protected:
	uint16 _gameThreadID;

private:
	bool _running = true;
	bool _isAlive = true;
	std::atomic<int> _runningThread = 0;
	LockFreeQueue<AsyncJob> _asyncJobQueue[ASYNC_JOG_THRAED_NUM];
	LockFreeQueue<AsyncJobResult> _asyncResultQueue;
	std::atomic<int> asyncThreadIndex = 0;
};

