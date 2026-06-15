#include "MonitorClient.h"
#include <process.h>
#include "SerializeBuffer.h"
#include "MonitorPacketMaker.h"
#include "MonitorPacket.h"

MonitorClient::MonitorClient()
{
	//TODO: Connect 하고
}

MonitorClient::~MonitorClient()
{
	WaitForSingleObject(_hUpdateThread, INFINITE);
	wprintf(L"MonitorClient destroyed\n");
}

//void MonitorClient::Run()
//{
//	wprintf(L"MonitorClient Run\n");
//	int errorCode;
//	_hUpdateThread = (HANDLE)_beginthreadex(NULL, 0, UpdateThreadStatic, this, 0, NULL);
//	if (_hUpdateThread == NULL)
//	{
//		wprintf(L"Failed to create update thread\n");
//		__debugbreak();
//	}
//}

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
	wprintf(L"Received packet from monitor server\n");
}

void MonitorClient::OnError(int errorCode, WCHAR* errorMessage)
{
	wprintf(L"Error from monitor server: %s\n", errorMessage);
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
