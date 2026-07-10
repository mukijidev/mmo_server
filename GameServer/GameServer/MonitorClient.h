#pragma once
#include "CLanClient.h"
#include "PerformanceMonitor.h"

class CPacket;
class GameServer;
class FieldPacketHandleThread;

class MonitorClient : public CLanClient
{
public:

	MonitorClient();
	~MonitorClient();

	//void Run();
	// 연결 완료시
	void OnConnect() override;
	// 연결 종료시
	void OnDisconnect() override;
	// 패킷 수신시
	void OnRecvPacket(CPacket* packet) override;
	// 에러 발생시
	void OnError(int errorCode, WCHAR* errorMessage) override;
	
	void SetGameServer(GameServer* server) { _gameServer = server; };
	void Run(); // 모니터클라 시작

private:
	PerformanceMonitor _performanceMonitor{ L"GameServer" };

	static unsigned int __stdcall UpdateThreadStatic(void* arg)
	{
		MonitorClient* pThis = (MonitorClient*)arg;
		pThis->UpdateThread();
		return 0;
	}

	unsigned int __stdcall UpdateThread();

	void SendMonitorSector(uint8 gridId, FieldPacketHandleThread* field);
	void SendMonitorData(uint8 dataType, int dataValue, int timeStamp);


public:
	HANDLE _hUpdateThread;
	GameServer* _gameServer = nullptr;
};

