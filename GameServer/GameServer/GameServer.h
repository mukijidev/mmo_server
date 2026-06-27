#pragma once
#include "CNetServer.h"
#include "LockFreeQueue.h"
#include "Player.h"
#include "MonitorClient.h"
#include "PerformanceMonitor.h"
#include <map>
#include "Log.h"

#include "GuardianFieldThread.h"
#include "LobbyFieldThread.h"
#include "SpiderFieldThread.h"
#include "LoginGameThread.h"
#include "GameData.h"

class GameServer : public CNetServer
{

public:
	GameServer();
	~GameServer();

private:
	// CLanServer을(를) 통해 상속됨
	virtual bool OnConnectionRequest() override;
	virtual void OnError(int errorCode, WCHAR* errorMessage) override;

private:
	static unsigned int __stdcall TimeOutThreadStatic(void* param)
	{
		GameServer* thisClass = (GameServer*)param;
		return thisClass->TimeOutThread();
	}
	unsigned int __stdcall TimeOutThread();

public:
	void LogServerInfo();
	void ObjectPoolLog();

private:
	HANDLE _hTimeOutThread;



private:
	MonitorClient* _monitorClient = nullptr;

private: // 모니터 서버로 보낼 거 (서버)
	int64 _loginTotal = 0;
	int64 _sectorMoveTotal = 0;
	int64 _messageTotal = 0;
	int _processUserAllocMemory;
	int _networkSend;
	int _networkRecv;

private: // 모니터 서버로 보낼 거 (하드웨어)
	PerformanceMonitor _performanceMonitor{ L"GameServerLib" };

private: // 플레이어
	SRWLOCK _playerMapLock;
	LockFreeObjectPool<class Player, true> _playerPool;
	std::unordered_map<int64, Player*> _globalPlayerMap;

private:
	LobbyFieldThread* _LobbyFieldThread = nullptr;
	SpiderFieldThread* _SpiderFieldThread = nullptr;
	GuardianFieldThread* _GuardianFieldThread = nullptr;
	LoginGameThread* _loginGameThread = nullptr;

public:
	Player* AllocPlayer(int64 sessionId)
	{
		Player* p = _playerPool.Alloc(nullptr, TYPE_PLAYER, sessionId);
		AcquireSRWLockExclusive(&_playerMapLock);
		auto ret = _globalPlayerMap.insert({ sessionId, p });
		if (ret.second == false)
		{
			// already exist
			LOG(L"GameServer", LogLevel::Error, L"Already exist sessionId : %lld, AllocPlayer", sessionId);
			ReleaseSRWLockExclusive(&_playerMapLock);
			_playerPool.Free(p);
			return nullptr;
		}
		ReleaseSRWLockExclusive(&_playerMapLock);
		return p;
	}

	void FreePlayer(int64 sessionId)
	{
		Player* p;
		AcquireSRWLockExclusive(&_playerMapLock);
		auto it = _globalPlayerMap.find(sessionId);
		if (it == _globalPlayerMap.end())
		{
			LOG(L"GameServer", LogLevel::Error, L"Cannot find sessionId : %lld, FreePlayer", sessionId);
			ReleaseSRWLockExclusive(&_playerMapLock);
			return;
		}
		p = (*it).second;
		_globalPlayerMap.erase(it);
		ReleaseSRWLockExclusive(&_playerMapLock);
		_playerPool.Free(p);
	}

private:
	uint16 clientPacketCode = Data::clientPacketCode;
	void MP_SS_MONITOR_LOGIN(CPacket* packet, int& serverNo);
	void MP_SC_MONITOR_TOOL_DATA_UPDATE(CPacket* packet, uint8& dataType, int& dataValue, int& timeStamp);

private:
	uint8** LoadMapData(std::string filePath, uint32 mapYSize, uint32 mapXSize);
	uint8** BuildCoarse(uint8** fine, int fineY, int fineX, int coarV, int& outNy, int& outNx);


	//Map
private:
	uint8** _lobbyMap = nullptr;
	uint8** _guardianMap = nullptr;
	uint8** _spiderMap = nullptr;

	uint8** _lobbyCoarse = nullptr;
	uint8** _guardianCoarse = nullptr;
	uint8** _spiderCoarse = nullptr;

	int _coarseY = 0;
	int _coarseX = 0;
};

