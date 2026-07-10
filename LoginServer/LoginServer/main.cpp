#pragma comment(lib, "ws2_32")
#pragma comment(lib, "Winmm.lib")
#pragma comment (lib, "cpp_redis.lib")
#pragma comment (lib, "tacopie.lib")
#pragma comment (lib, "ws2_32.lib")
#pragma comment(lib, "mysqlclient.lib")

#include <conio.h>
#include <stdio.h>
#include "Data.h"
#include <clocale>
#include "Type.h"
#include "LoginServer.h"
#include "TlsObjectPool.h"
#include "Profiler.h"
#include "Log.h"


int main()
{
	timeBeginPeriod(1);
	//데이터 로드
	bool loadDataSucceed = Data::LoadData();
	if (!loadDataSucceed)
	{
		wprintf(L"load data failed\n");
		return 0;
	}


	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		wprintf(L"wsa start up failed\n");
		return false;
	}


	// 로그 초기화 
	INIT_SYSLOG(L"Log"); //폴더 이름 Log
	SYSLOG_LEVEL(LogLevel(Data::LogLevel));

	//프로파일링 초기화
	ProfileInit();


	//채팅 서버 시작
	LoginServer* loginServer = new LoginServer;
	bool startSuccess = loginServer->Start(Data::ServerPort, Data::ServerConcurrentThreadNum,
		Data::ServerWorkerThreadNum, Data::Nagle, Data::SendZeroCopy);
	if (!startSuccess)
	{
		LOG(L"System", LogLevel::System, L"Start Chatting Server Failed");
		return 0;
	}
	LOG(L"System", LogLevel::System, L"Start Chatting Server Succeed");

	//모니터 서버에 연결시 모니터 클라이언트 활성화
	if (Data::MonitorActivate)
	{
		std::wstring wMonitorServerIp;
		wMonitorServerIp.assign(Data::MonitorServerIp.begin(), Data::MonitorServerIp.end());
		bool bActivateMonitorClinent = loginServer->ActivateMonitorClient(wMonitorServerIp.c_str(), Data::MonitorPort, Data::MonitorClientConcurrentThreadNum, Data::MonitorClientWorkerThreadNum);
		if (!bActivateMonitorClinent)
		{
			LOG(L"System", LogLevel::System, L"Connect Monitor Client Failed");
			//return false;
		}
	}

	int logTime = timeGetTime();
	int logDuration = 1000;
	while (true)
	{
		Sleep(999);
		int now = timeGetTime();
		if (now - logTime > logDuration)
		{
			//1초당 로그찍기
			loginServer->LogServerInfo();
			// Packet TLS풀 로그찍기
			//loginServer->ObjectPoolLog();
			logTime += logDuration;
		}
		
		if (_kbhit())
		{
			//서버 종료
			WCHAR controlKey = _getwch();
			if (L'q' == controlKey || L'Q' == controlKey)
			{
				LOG(L"System", LogLevel::System, L"Server Stop By Keyboard");
				loginServer->Stop();
				break;
			}else if (L'c' == controlKey || L'C' == controlKey)
			{
				// 덤프 남기기
				LOG(L"System", LogLevel::System, L"Crash By Keyboard");
				__debugbreak();
			}
			else if (L's' == controlKey || L'S' == controlKey)
			{
				PRO_SAVE(L"profile");
			}
			else if (L'z' == controlKey || L'Z' == controlKey)
			{
				PRO_RESET();
			}
		}
	}

	delete loginServer;
	LOG(L"System", LogLevel::System, L"Main Thread Exit");
	WSACleanup();
	return 0;
}