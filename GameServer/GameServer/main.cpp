#pragma comment(lib, "ws2_32")
#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "mysqlclient.lib")

#include <conio.h>
#include <stdio.h>
#include "Data.h"
#include <clocale>
#include "Type.h"
#include "GameServer.h"
#include "TlsObjectPool.h"
#include "Profiler.h"
#include "Log.h"

static bool LoadData()
{
	if (!Data::LoadData())
	{
		wprintf(L"Load Data Failed\n");
		return false;
	}
	return true;
}

static bool InitNetwork()
{
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		wprintf(L"wsa start up failed\n");
		return false;
	}
	return true;
}

static void InitLog()
{
	INIT_SYSLOG(L"Log"); //폴더 이름 Log
	SYSLOG_LEVEL(LogLevel(Data::LogLevel));
	LOG(L"test", LogLevel::Error, L"error");
}

static GameServer* StartServer()
{
	GameServer* gameServer = new GameServer;
	bool startSuccess = gameServer->Start(Data::ServerPort, Data::ServerConcurrentThreadNum,
		Data::ServerWorkerThreadNum, Data::Nagle, Data::SendZeroCopy);
	if (!startSuccess)
	{
		LOG(L"System", LogLevel::System, L"Start GameServer Failed");
		delete gameServer;
		return nullptr;
	}
	LOG(L"System", LogLevel::System, L"Start GameServer Succeed");
	return gameServer;
}

int main()
{
	timeBeginPeriod(1);
	//데이터 로드
	if (!LoadData())
		return 0;
	if (!InitNetwork())
		return 0;

	// 로그 초기화 
	InitLog();
	//프로파일링 초기화
	ProfileInit();
	//게임 서버 시작
	GameServer* gameServer = StartServer();
	if (!gameServer)
		return 0;

	int logTime = timeGetTime();
	int logDuration = 1000;
	while (true)
	{
		Sleep(999);
		int now = timeGetTime();
		if (now - logTime > logDuration)
		{
			//1초당 로그찍기
			gameServer->LogServerInfo();
			// Packet TLS풀 로그찍기
			//gameServer->ObjectPoolLog();

			logTime += logDuration;
		}
		
		if (_kbhit())
		{
			//서버 종료
			WCHAR controlKey = _getwch();
			if (L'q' == controlKey || L'Q' == controlKey)
			{
				LOG(L"System", LogLevel::System, L"Server Stop By Keyboard");
				gameServer->Stop();
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

	delete gameServer;
	timeEndPeriod(1);
	LOG(L"System", LogLevel::System, L"Main Thread Exit");
	WSACleanup();
	return 0;
}