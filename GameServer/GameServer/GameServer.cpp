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

	_LobbyFieldThread = new LobbyFieldThread(this, FIELD_LOBBY, 100, 15, 15, 800, 800, _lobbyMap);
	_GuardianFieldThread = new GuardianFieldThread(this, FIELD_GUARDIAN, 100, 15, 15, 800, 800, _guardianMap);
	_SpiderFieldThread = new SpiderFieldThread(this, FIELD_SPIDER, 100, 15, 15, 800, 800, _spiderMap);
	
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
	int gamePlayerNum = _GuardianFieldThread->GetPlayerSize();
	int64 totalPlayerNum = _globalPlayerMap.size();
	int loginTps = _loginGameThread->GetFps();
	int fieldThreadTPS = _LobbyFieldThread->GetFps();
	int64 jpsTps = _LobbyFieldThread->GetJPSFps();
	int64 jpsQueueSize = _LobbyFieldThread->GetJPSQueueSize();

	printf("Total Accept : %lld\n\
Accept TPS : % lld\n\
Send Message TPS : %lld\n\
Recv Message TPS : %lld\n\
session num :%lld\n\
login : %d\n\
game: %d\n\
total: %lld\n\
total disconnect : %lld\n\
disconnect by sendqueue full : %lld\n\
memory : %d\n\
send: %d\n\
recv: %d\n\
logintTps : %d\n\
fieldTps : %d\n\
jpsTps : %lld\n\
jpsQueueSize : %lld\n\
================\n\
", totalAcceptValue, acceptTPSValue, sendMessageTPSValue, recvMessageTPSValue, sessionNum, loginPlayerNum, gamePlayerNum, totalPlayerNum,
totalDisconnect, disconnectBySendQueueFull, processUserAllocMemory, networkSend, networkRecv, loginTps, fieldThreadTPS, jpsTps, jpsQueueSize);
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




bool GameServer::OnConnectionRequest()
{
	// 이거 GameServer
	return false;
}

void GameServer::OnError(int errorCode, WCHAR* errorMessage)
{
	// 이거는 ??? 
}
