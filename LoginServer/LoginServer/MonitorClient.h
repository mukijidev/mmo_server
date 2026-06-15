#pragma once
#include "CLanClient.h"


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


	//static unsigned int __stdcall UpdateThreadStatic(void* arg)
	//{
	//	MonitorClient* pThis = (MonitorClient*)arg;
	//	pThis->UpdateThread();
	//	return 0;
	//}

	//unsigned int __stdcall UpdateThread();

public:
	HANDLE _hUpdateThread;
};

