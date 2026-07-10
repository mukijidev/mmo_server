#include "GameServer.h"
#include "SerializeBuffer.h"
#include "PacketHeader.h"
#include <process.h>
#include "TlsObjectPool.h"
#include "Packet.h"
#include "Profiler.h"
#include "ObjectPool.h"
#include <inttypes.h>
#include "LockFreeObjectPool.h"
#include "TlsObjectPool.h"
#include "Log.h"
#include "MonitorPacketMaker.h"
#include "PerformanceMonitor.h"
#include "algorithm"
#include "GuardianFieldThread.h"
#include "LoginGameThread.h"
#include "CpuUsage.h"
#include <time.h>
#include "GameData.h"
//#define _LOG
//같은쓰레드를 여러개 만들면?


using namespace std;
GameServer::GameServer()
{
	InitializeSRWLock(&_playerMapLock);
	_loginGameThread = new LoginGameThread(this, LOGIN_THREAD, 100);
	//TODO: 여기서 맵 로딩하고
	//TODO: GameThread만들때 맵데이터 넘기고
	
	_lobbyMap = LoadMapData("map/LobbyMap.dat", 12000, 12000);
	_guardianMap = LoadMapData("map/GuardianMap.dat", 12000, 12000);
	_spiderMap = LoadMapData("map/SpiderMap.dat", 12000, 12000);

	_lobbyCoarse = BuildCoarse(_lobbyMap, 12000, 12000, COARSE_CELL, _coarseY, _coarseX);
	_guardianCoarse = BuildCoarse(_guardianMap, 12000, 12000, COARSE_CELL, _coarseY, _coarseX);
	_spiderCoarse = BuildCoarse(_spiderMap, 12000, 12000, COARSE_CELL, _coarseY, _coarseX);



	_LobbyFieldThread = new LobbyFieldThread(this, FIELD_LOBBY, 100, 15, 15, 800, 800, _lobbyMap, _lobbyCoarse);
	_GuardianFieldThread = new GuardianFieldThread(this, FIELD_GUARDIAN, 100, 15, 15, 800, 800, _guardianMap, _guardianCoarse);
	_SpiderFieldThread = new SpiderFieldThread(this, FIELD_SPIDER, 100, 15, 15, 800, 800, _spiderMap, _spiderCoarse);
	
	_loginGameThread->Start();
	_LobbyFieldThread->Start();
	_GuardianFieldThread->Start();
	_SpiderFieldThread->Start();

	RegisterDefaultGameThread(_loginGameThread);
	LOG(L"System", LogLevel::System, L"GameServer()");
}

GameServer::~GameServer()
{
	_GuardianFieldThread->Stop();
	_loginGameThread->Stop();
	_LobbyFieldThread->Stop();
	_SpiderFieldThread->Stop();

	Sleep(1000);
	bool bEchoDead = _GuardianFieldThread->CheckDead();
	if (!bEchoDead)
	{
		int64 sessionNum = _GuardianFieldThread->GetSessionNum();
		LOG(L"EchoThread", LogLevel::Error, L"Session :%lld (num) is left", sessionNum);
		_GuardianFieldThread->Kill();
	}

	bool bLoginDead = _loginGameThread->CheckDead();
	if (!bLoginDead)
	{
		int64 sessionNum = _loginGameThread->GetSessionNum();
		LOG(L"LoginThread", LogLevel::Error, L"Session :%lld (num) is left", sessionNum);
		_loginGameThread->Kill();
	}

	bool bLobbyDead = _LobbyFieldThread->CheckDead();
	if (!bLobbyDead)
	{
		int64 sessionNum = _LobbyFieldThread->GetSessionNum();
		LOG(L"LobbyThread", LogLevel::Error, L"Session :%lld (num) is left", sessionNum);
		_LobbyFieldThread->Kill();
	}

	bool bSpiderDead = _SpiderFieldThread->CheckDead();
	if (!bSpiderDead)
	{
		int64 sessionNum = _SpiderFieldThread->GetSessionNum();
		LOG(L"SpiderThread", LogLevel::Error, L"Session :%lld (num) is left", sessionNum);
		_SpiderFieldThread->Kill();
	}

	delete _GuardianFieldThread;
	delete _loginGameThread;
	delete _LobbyFieldThread;
	delete _SpiderFieldThread;

	for (int i = 0; i < 12000; i++)
	{
		delete[] _lobbyMap[i];
		delete[] _guardianMap[i];
		delete[] _spiderMap[i];
	}

	delete[] _lobbyMap;
	delete[] _guardianMap;
	delete[] _spiderMap;

	LOG(L"System", LogLevel::System, L"~GameServer()");
}

unsigned int __stdcall GameServer::TimeOutThread()
{
	while (true)
	{
		Sleep(1000);
	}

	return 0;
}

void GameServer::LogServerInfo()
{
	int64 totalAcceptValue = GetTotalAcceptValue();
	int64 acceptTPSValue = GetAcceptTPS();
	int64 sendMessageTPSValue = GetSendMessageTPS();
	int64 recvMessageTPSValue = GetRecvMessageTPS();
	int64 totalDisconnect = GetTotalDisconnect();
	int64 sessionNum = GetSessionNum();
	int64 disconnectBySendQueueFull = GetDisconnectSessionNumBySendQueueFull();
	int processUserAllocMemory = _processUserAllocMemory;
	int networkSend = _networkSend;
	int networkRecv = _networkRecv;

	int loginPlayerNum = _loginGameThread->GetPlayerSize();
	int guardianPlayerNum = _GuardianFieldThread->GetPlayerSize();
	int lobbyPlayerNum = _LobbyFieldThread->GetPlayerSize();
	int spiderPlayerNum = _SpiderFieldThread->GetPlayerSize();
	int64 totalPlayerNum = _globalPlayerMap.size();
	int loginTps = _loginGameThread->GetFps();
	int fieldThreadTPS = _LobbyFieldThread->GetFps();

	int64 lobbyJpsTps = _LobbyFieldThread->GetJPSFps();
	int64 lobbyJpsQueueSize = _LobbyFieldThread->GetJPSQueueSize();
	int64 guardianJpsTps = _GuardianFieldThread->GetJPSFps();
	int64 guardianJpsQueueSize = _GuardianFieldThread->GetJPSQueueSize();
	int64 spiderJpsTps = _SpiderFieldThread->GetJPSFps();
	int64 spiderJpsQueueSize = _SpiderFieldThread->GetJPSQueueSize();

	printf("Total Accept : %lld\n\
Accept TPS : % lld\n\
Send Message TPS : %lld\n\
Recv Message TPS : %lld\n\
session num :%lld\n\
login : %d\n\
lobby : %d\n\
guardian: %d\n\
spider: %d\n\
total: %lld\n\
total disconnect : %lld\n\
disconnect by sendqueue full : %lld\n\
memory : %d\n\
send: %d\n\
recv: %d\n\
logintTps : %d\n\
fieldTps : %d\n\
[LOBBY]    jpsTps : %lld  queue : %lld\n\
[GUARDIAN] jpsTps : %lld  queue : %lld\n\
[SPIDER]   jpsTps : %lld  queue : %lld\n\
================\n\
", totalAcceptValue, acceptTPSValue, sendMessageTPSValue, recvMessageTPSValue, sessionNum, loginPlayerNum, lobbyPlayerNum, guardianPlayerNum, spiderPlayerNum, totalPlayerNum,
totalDisconnect, disconnectBySendQueueFull, processUserAllocMemory, networkSend, networkRecv, loginTps, fieldThreadTPS,
lobbyJpsTps, lobbyJpsQueueSize, guardianJpsTps, guardianJpsQueueSize, spiderJpsTps, spiderJpsQueueSize);

	//_LobbyFieldThread->LogAndResetFindPathDebug("LOBBY");
	//_GuardianFieldThread->LogAndResetFindPathDebug("GUARDIAN");
	//_SpiderFieldThread->LogAndResetFindPathDebug("SPIDER");

}


void GameServer::ObjectPoolLog()
{
	printf("packetusecount : %lld\n", CPacket::GetUseCount());
	printf("PlayerPool : capaicty : %d useCount : %d\n", _playerPool.GetCapacityCount(), _playerPool.GetUseCount());
	for (int i = 0; i < MAX_TLS_LOG_INDEX; i++)
	{
		if (_objectPoolMonitor[i].threadId == 0)
			break;

		TlsLog& tlsLog = _objectPoolMonitor[i];


		printf("thread id : %d , size = %d, mallocCount : %d\n", tlsLog.threadId, tlsLog.size, tlsLog.mallocCount);
	}

	printf("stack size : %d\n", releaseStackSize);
}

bool GameServer::ActivateMonitorClient(const WCHAR* serverIp, uint16 port, uint32 concurrentThreadNum, uint32 workerThreadNum)
{
	_monitorClient = new MonitorClient();

	bool bStart = _monitorClient->Start(serverIp, port, concurrentThreadNum, workerThreadNum);
	if (!bStart)
		return false;

	bool bConnect = _monitorClient->Connect();
	if (!bConnect)
		return false;

	_monitorClient->SetGameServer(this);
	_monitorClient->Run();

	return true;

}




bool GameServer::OnConnectionRequest()
{
	// 이거 GameServer
	return false;
}

void GameServer::OnError(int errorCode, WCHAR* errorMessage)
{
	// 이거는 ??? 
}


uint8** GameServer::LoadMapData(string filePath, uint32 mapYSize, uint32 mapXSize)
{
	//TODO: lobbymap.data 가져와서	
	uint8** map = new uint8 * [mapYSize];
	for (uint32 i = 0; i < mapYSize; i++)
	{
		map[i] = new uint8[mapXSize];
	}

	FILE* f;
	fopen_s(&f, filePath.c_str(), "rb");
	if (f == nullptr)
	{
		printf("Cannot open file : %s\n", filePath.c_str());
		LOG(L"GameServer", LogLevel::Error, L"Cannot open file : %s", filePath.c_str());
		return nullptr;
	}

	for (uint32 i = 0; i < mapYSize; i++)
	{
		fread(map[i], sizeof(uint8), mapXSize, f);
	}

	fclose(f);
	return map;
}

uint8** GameServer::BuildCoarse(uint8** fine, int fineY, int fineX, int coarV, int& outNy, int& outNx)
{
	int nY = fineY / coarV;
	int nX = fineX / coarV;

	uint8** coarse = new uint8 * [nY];
	for (int cy = 0; cy < nY; ++cy)
	{
		coarse[cy] = new uint8[nX];
		for (int cx = 0; cx < nX; ++cx)
		{
			bool bObs = false;
			for (int dy = 0; dy < coarV && !bObs; ++dy)
				for (int dx = 0; dx < coarV; ++dx)
					if (fine[cy * coarV + dy][cx * coarV + dx] == OBSTACLE)
					{
						bObs = true;
						break;
					}

			if (bObs)

			{
				coarse[cy][cx] = OBSTACLE;
			}
			else {
				coarse[cy][cx] = NONE;
			}
		}
	}

	outNy = nY;
	outNx = nX;

	return coarse;
}