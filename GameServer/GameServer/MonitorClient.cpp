#include "MonitorClient.h"
#include <process.h>
#include "SerializeBuffer.h"
#include "MonitorPacketMaker.h"
#include "MonitorPacket.h"
#include "GameServer.h"
#include "MonitorProtocol.h"
#include "CpuUsage.h"


MonitorClient::MonitorClient()
{
	//TODO: Connect 하고
}

MonitorClient::~MonitorClient()
{
	WaitForSingleObject(_hUpdateThread, INFINITE);
	CloseHandle(_hUpdateThread);
	wprintf(L"MonitorClient destroyed\n");
}

void MonitorClient::Run()
{
	wprintf(L"MonitorClient Run\n");
	int errorCode;
	_hUpdateThread = (HANDLE)_beginthreadex(NULL, 0, UpdateThreadStatic, this, 0, NULL);
	if (_hUpdateThread == NULL)
	{
		wprintf(L"Failed to create update thread\n");
		__debugbreak();
	}
}

void MonitorClient::OnConnect()
{
	wprintf(L"Connected to monitor server\n");
}

void MonitorClient::OnDisconnect()
{
	wprintf(L"Disconnected from monitor server\n");
}

void MonitorClient::OnRecvPacket(CPacket* packet)
{
	wprintf(L"Receive packet from monitor server\n");
}

void MonitorClient::OnError(int errorCode, WCHAR* errorMessage)
{
	wprintf(L"Monitor server Error: %s\n", errorMessage);
}

void MonitorClient::SendMonitorData(uint8 dataType, int dataValue, int timeStamp)
{
	CPacket* p = CPacket::Alloc();
	MP_SS_MONITOR_DATA_UPDATE(p, dataType, dataValue, timeStamp);
	SendPacket(p);
	CPacket::Free(p);
}

void MonitorClient::SendMonitorSector(uint8 gridId, FieldPacketHandleThread* field)
{
	uint8 cells[MONITOR_SECTOR_GRID * MONITOR_SECTOR_GRID];
	field->CopyMonitorSectorCells(cells);

	CPacket* p = CPacket::Alloc();
	MP_SS_MONITOR_SECTOR(p, gridId, MONITOR_SECTOR_GRID, MONITOR_SECTOR_GRID, cells);
	SendPacket(p);
	CPacket::Free(p);
}

unsigned int __stdcall MonitorClient::UpdateThread()
{
	{
		CPacket* loginPacket = CPacket::Alloc();
		MP_SS_MONITOR_LOGIN(loginPacket, SERVER_NO_GAME);
		SendPacket(loginPacket);
		CPacket::Free(loginPacket);
	}

	CpuUsage cpuUsage;
	PerformanceMonitorData perfData;


	while (!_gameServer->isNetworkStop())
	{
		Sleep(1000);
		int64 llTick;
		time(&llTick);
		int ts = (int)llTick;

	
		int packetUseCount = (int)CPacket::GetUseCount();
		//hardware
		cpuUsage.UpdateCpuTime();
		_performanceMonitor.Update(perfData);

		SendMonitorData(dfMONITOR_DATA_TYPE_SERVER_CPU, (int)cpuUsage.ProcessorTotal(), ts);
		SendMonitorData(dfMONITOR_DATA_TYPE_SERVER_AVAILABLE_MEM, (int)perfData.availableMemoryCounterVal.doubleValue, ts);
		SendMonitorData(dfMONITOR_DATA_TYPE_SERVER_NONPAGED_MEM, (int)(perfData.nonPageMemoryCounterVal.doubleValue / (1024.0 * 1024.0)), ts);

		int networkRecv = (int)(_gameServer->GetNetworkRecvByteTps() / 1024);
		int networkSend = (int)(_gameServer->GetNetworkSendByteTps() / 1024);
		_gameServer->_networkRecv = networkRecv;
		_gameServer->_networkSend = networkSend;
		SendMonitorData(dfMONITOR_DATA_TYPE_SERVER_NETWORK_RECV, networkRecv, ts);
		SendMonitorData(dfMONITOR_DATA_TYPE_SERVER_NETWORK_SEND, networkSend, ts);

		// gameserver
		SendMonitorData(dfMONITOR_DATA_TYPE_GAME_SERVER_RUN, 1, ts);
		SendMonitorData(dfMONITOR_DATA_TYPE_GAME_SERVER_CPU, (int)cpuUsage.ProcessorTotal(), ts);
		int processMem = (int)(perfData.processUserAllocMemoryCounterVal.doubleValue / (1024.0 * 1024.0));
		_gameServer->_processUserAllocMemory = processMem;
		SendMonitorData(dfMONITOR_DATA_TYPE_GAME_SERVER_MEM, processMem, ts);
		SendMonitorData(dfMONITOR_DATA_TYPE_GAME_PACKET_POOL, packetUseCount, ts);
		SendMonitorData(dfMONITOR_DATA_TYPE_GAME_ACCEPT_TPS, (int)_gameServer->GetAcceptTPS(), ts);
		SendMonitorData(dfMONITOR_DATA_TYPE_GAME_SEND_MSG_TPS, (int)_gameServer->GetSendMessageTPS(), ts);
		SendMonitorData(dfMONITOR_DATA_TYPE_GAME_RECV_MSG_TPS, (int)_gameServer->GetRecvMessageTPS(), ts);
		SendMonitorData(dfMONITOR_DATA_TYPE_GAME_TOTAL_DISCONNECT, (int)_gameServer->GetTotalDisconnect(), ts);
		SendMonitorData(dfMONITOR_DATA_TYPE_GAME_DISCONNECT_SENDQ_FULL, (int)_gameServer->GetDisconnectSessionNumBySendQueueFull(), ts);
		SendMonitorData(dfMONITOR_DATA_TYPE_GAME_AUTH_PLAYER, (int)_gameServer->_loginGameThread->GetPlayerSize(), ts);
		SendMonitorData(dfMONITOR_DATA_TYPE_GAME_GAME_PLAYER, (int)_gameServer->_globalPlayerMap.size(), ts);
		SendMonitorData(dfMONITOR_DATA_TYPE_GAME_SESSION, (int)_gameServer->GetSessionNum(), ts);
		SendMonitorData(dfMONITOR_DATA_TYPE_GAME_AUTH_THREAD_FPS, (int)_gameServer->_loginGameThread->GetFps(), ts);
		SendMonitorData(dfMONITOR_DATA_TYPE_GAME_GAME_THREAD_FPS, (int)_gameServer->_LobbyFieldThread->GetFps(), ts);

		//DB
		int dbTps = _gameServer->_LobbyFieldThread->GetDbWriteTps() + _gameServer->_GuardianFieldThread->GetDbWriteTps()
			+ _gameServer->_SpiderFieldThread->GetDbWriteTps();
		int dbQueue = _gameServer->_LobbyFieldThread->GetDBQueueSize() + _gameServer->_GuardianFieldThread->GetJPSQueueSize()
			+ _gameServer->_SpiderFieldThread->GetJPSQueueSize();

		SendMonitorData(dfMONITOR_DATA_TYPE_GAME_DB_TPS, dbTps, ts);
		SendMonitorData(dfMONITOR_DATA_TYPE_GAME_DB_QUEUE, dbQueue, ts);

		//JPS
		SendMonitorData(dfMONITOR_DATA_TYPE_GAME_JPS_LOBBY_PLAYER_TPS, _gameServer->_LobbyFieldThread->GetPlayerJpsTps(), ts);
		SendMonitorData(dfMONITOR_DATA_TYPE_GAME_JPS_LOBBY_MONSTER_TPS, _gameServer->_LobbyFieldThread->GetMonsterJpsTps(), ts);
		SendMonitorData(dfMONITOR_DATA_TYPE_GAME_JPS_GUARDIAN_PLAYER_TPS, _gameServer->_GuardianFieldThread->GetPlayerJpsTps(), ts);
		SendMonitorData(dfMONITOR_DATA_TYPE_GAME_JPS_GUARDIAN_MONSTER_TPS, _gameServer->_GuardianFieldThread->GetMonsterJpsTps(), ts);
		SendMonitorData(dfMONITOR_DATA_TYPE_GAME_JPS_SPIDER_PLAYER_TPS, _gameServer->_SpiderFieldThread->GetPlayerJpsTps(), ts);
		SendMonitorData(dfMONITOR_DATA_TYPE_GAME_JPS_SPIDER_MONSTER_TPS, _gameServer->_SpiderFieldThread->GetMonsterJpsTps(), ts);
		SendMonitorData(dfMONITOR_DATA_TYPE_GAME_JPS_LOBBY_PLAYER_QUEUE, (int)_gameServer->_LobbyFieldThread->GetPlayerJpsQueueSize(), ts);
		SendMonitorData(dfMONITOR_DATA_TYPE_GAME_JPS_LOBBY_MONSTER_QUEUE, (int)_gameServer->_LobbyFieldThread->GetMonsterJpsQueueSize(), ts);
		SendMonitorData(dfMONITOR_DATA_TYPE_GAME_JPS_GUARDIAN_PLAYER_QUEUE, (int)_gameServer->_GuardianFieldThread->GetPlayerJpsQueueSize(), ts);
		SendMonitorData(dfMONITOR_DATA_TYPE_GAME_JPS_GUARDIAN_MONSTER_QUEUE, (int)_gameServer->_GuardianFieldThread->GetMonsterJpsQueueSize(), ts);
		SendMonitorData(dfMONITOR_DATA_TYPE_GAME_JPS_SPIDER_PLAYER_QUEUE, (int)_gameServer->_SpiderFieldThread->GetPlayerJpsQueueSize(), ts);
		SendMonitorData(dfMONITOR_DATA_TYPE_GAME_JPS_SPIDER_MONSTER_QUEUE, (int)_gameServer->_SpiderFieldThread->GetMonsterJpsQueueSize(), ts);


		//frame
		SendMonitorData(dfMONITOR_DATA_TYPE_GAME_FRAME_AVG, (int)_gameServer->_LobbyFieldThread->GetFrameAvgMs(), ts);
		SendMonitorData(dfMONITOR_DATA_TYPE_GAME_FRAME_MAX, (int)_gameServer->_LobbyFieldThread->GetFrameMaxMs(), ts);

		// monster
		SendMonitorData(dfMONITOR_DATA_TYPE_GAME_MONSTER_GUARDIAN_ALIVE, _gameServer->_GuardianFieldThread->GetAliveMonsterNum(), ts);
		SendMonitorData(dfMONITOR_DATA_TYPE_GAME_MONSTER_GUARDIAN_MAX, _gameServer->_GuardianFieldThread->GetMaxMonsterNum(), ts);
		SendMonitorData(dfMONITOR_DATA_TYPE_GAME_MONSTER_SPIDER_ALIVE, _gameServer->_SpiderFieldThread->GetAliveMonsterNum(), ts);
		SendMonitorData(dfMONITOR_DATA_TYPE_GAME_MONSTER_SPIDER_MAX, _gameServer->_SpiderFieldThread->GetMaxMonsterNum(), ts);

		// sector
		SendMonitorSector(SECTOR_GRID_LOBBY, _gameServer->_LobbyFieldThread);
		SendMonitorSector(SECTOR_GRID_GUARDIAN, _gameServer->_GuardianFieldThread);
		SendMonitorSector(SECTOR_GRID_SPIDER, _gameServer->_SpiderFieldThread);

	}

	return 0;
}

// 2500 * 2
//unsigned int __stdcall MonitorClient::UpdateThread()
//{
//	//TODO: 파이썬 클라이언트 구현 및 보내고 그림 그리기
//
//	uint16 sessionNumPerSector[50][50];
//
//
//	while (true)
//	{
//		for (int y = 0; y < 50; y++)
//		{
//			for (int x = 0; x < 50; x++)
//			{
//				sessionNumPerSector[y][x] = rand() % 10;
//			}
//		}
//
//
//
//		double cpuUsageTotal = 30.0f + rand() % 30;
//		double cpuUsageUser = 5.0f + rand() % 5;
//		double cpuUsageKernel = 25.f + rand() % 25;
//
//		double processUserAllocMemory = 100.f + rand() % 100;
//		double processNonPageMemory = 200.f + rand() % 200;
//		double availableMemory = 300.f + rand() % 300;
//		double nonPagedMemory = 400.f + rand() % 400;
//
//		Sleep(1000);
//		CPacket* packet = CPacket::Alloc();
//
//		MP_CS_MONITOR_VALUE(packet, sessionNumPerSector,
//			cpuUsageTotal, cpuUsageUser, cpuUsageKernel,
//			processUserAllocMemory, processNonPageMemory, availableMemory, nonPagedMemory);
//
//		SendPacket(packet);
//		CPacket::Free(packet);
//	}
//
//	return 0;
//}
