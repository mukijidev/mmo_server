#include "ChattingServer.h"
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
#include <time.h>
#include "CpuUsage.h"

//#define _LOG

using namespace std;
ChattingServer::ChattingServer()
{
	_hEvent =  CreateEvent(
		NULL, // default security attributes
		FALSE, // auto reset event
		FALSE, // initial state is nonsignaled
		NULL); // object name

	int errorCode;

	_hWorkerThread = (HANDLE)_beginthreadex(NULL, 0, UpdateThreadStatic, this, 0, NULL);
	if (_hWorkerThread == NULL)
	{
		errorCode = WSAGetLastError();
		__debugbreak();
	}

	//SetThreadPriority(_hWorkerThread, THREAD_PRIORITY_HIGHEST);

	_hTimeOutThread = (HANDLE)_beginthreadex(NULL, 0, TimeOutThreadStatic, this, 0, NULL);
	if (_hTimeOutThread == NULL)
	{
		errorCode = WSAGetLastError();
		__debugbreak();
	}

	_hMonitorThread = (HANDLE)_beginthreadex(NULL, 0, MonitorThreadStatic, this, 0, NULL);
	if (_hMonitorThread == NULL)
	{
		errorCode = WSAGetLastError();
		__debugbreak();
	}
	
	LOG(L"System", LogLevel::System, L"ChattingServer()");
}

ChattingServer::~ChattingServer()
{
	WaitForSingleObject(_hWorkerThread, INFINITE);
	WaitForSingleObject(_hTimeOutThread, INFINITE);
	if (_monitorActivated)
	{
		WaitForSingleObject(_hMonitorSendThread, INFINITE);
	}
	LOG(L"System", LogLevel::System, L"~ChattingServer()");
}

bool ChattingServer::OnConnectionRequest()
{
	//virtual bool OnConnectionRequest(IP, Port) = 0; < accept 직후
	/*return false; 시 클라이언트 거부.
	return true; 시 접속 허용*/
	return true;
}

void ChattingServer::OnAccept(int64 sessionId)
{
	_packetQueue.Enqueue({ true, NET_MESSAGE_ACCEPT, sessionId, nullptr });
	SetEvent(_hEvent);
}


inline void ChattingServer::OnDisconnect(int64 sessionId)
{
	_packetQueue.Enqueue({ 1, NET_MESSAGE_DISCONNECT, sessionId, nullptr });
	SetEvent(_hEvent);
}

inline void ChattingServer::OnRecvPacket(int64 sessionId, CPacket* packet)
{
	uint16 jobType;
	*packet >> jobType;
	_packetQueue.Enqueue({ 0, jobType, sessionId, packet });
	SetEvent(_hEvent);
}


void ChattingServer::OnError(int errorCode, WCHAR* errorMessage)
{

}


unsigned int __stdcall ChattingServer::UpdateThread()
{
	Job job;


	int compare = 0;
	while (!isNetworkStop())
	{
		WaitForSingleObject(_hEvent, INFINITE);

		for(;;)
		{
			if (!_packetQueue.Dequeue(job))
			{
				break;
			}
			int64 sessionId = job._sessionId;
			CPacket* packet = job._packet;
			int64 jobType = job._jobType;


			switch(jobType)
			{
			
			case PACKET_CS_CHAT_REQ_LOGIN:
			{
				//하여튼 그냥 player에 저장해두고
				PRO_BEGIN(L"HandleLogin");
				HandleLogin(sessionId, packet);
				CPacket::Free(packet);
				PRO_END(L"HandleLogin");

			}
			break;

			case PACKET_CS_CHAT_REQ_MESSAGE:
			{
				// 채팅메시지
				// TODO: login message안왔는데 보내면 안되고 (끊어버리고
				// sector_move안왔는데 보내면 안되고 (무시하고)
				PRO_BEGIN(L"HandleMessage");
				HandleMessage(sessionId, packet);
				CPacket::Free(packet);
				PRO_END(L"HandleMessage");
			}
			break;


			case 3:
			{
				PRO_BEGIN(L"HandleHeartBeat");
				HandleHeartBeat(sessionId, packet);
				CPacket::Free(packet);
				PRO_END(L"HandleHeartBeat");
			}
			break;


			case NET_MESSAGE_ACCEPT:
			{
				PRO_BEGIN(L"HandleAccept");
				HandleAccept(sessionId);
				PRO_END(L"HandleAccept");

			}
			break;

			case NET_MESSAGE_DISCONNECT:
			{
				PRO_BEGIN(L"HandleDisconnect");
				HandleDisconnect(sessionId);
				PRO_END(L"HandleDisconnect");
			}
			break;


			case NET_MESSAGE_TIMEOUT:
			{
				PRO_BEGIN(L"HandleTimeOut");
				HandleTimeOut();
				PRO_END(L"HandleTimeOut");
			}
			break;

		
			default:
				__debugbreak();

			}

			_updateTps++;
		}
	}
	return 0;
}


unsigned int __stdcall ChattingServer::TimeOutThread()
{
	Job job{ true, NET_MESSAGE_TIMEOUT, 0, nullptr };
	while (!isNetworkStop())
	{
		/*Job* job = JobPool.Alloc();
		job->Init(true, NET_MESSAGE_TIMEOUT, 0, nullptr);*/
		Sleep(TIMEOUT_DISCONNECT);
	/*	_packetQueue.Enqueue(job);
		SetEvent(_hEvent);*/
	}
	return 0;
}

void ChattingServer::HandleTimeOut()
{
	uint32 now = timeGetTime();
	for (auto it = _playerMap.begin(); it != _playerMap.end(); ++it)
	{
		Player* player = (*it).second;
		if (now - player->_lastRecvTime > TIMEOUT_DISCONNECT)
		{
			Disconnect(player->_sessionId);
		}
	}
}


void ChattingServer::HandleAccept(int64 sessionId)
{
	//TODO: player생성
	Player* player = _playerPool.Alloc();
	player->Init(sessionId);
	player->_lastRecvTime = timeGetTime();

	auto ret = _playerMap.insert({ sessionId, player});
	if (ret.second == false)
	{
		// already exist
		__debugbreak();
	}
	
}



void ChattingServer::HandleDisconnect(int64 sessionId)
{
	auto playerIt = _playerMap.find(sessionId);
	if (playerIt == _playerMap.end())
	{
		__debugbreak();
	}

	Player* player = (*playerIt).second;


	// 맵에서 제거하고
	_playerMap.erase(playerIt);
	
	// 풀에 반납
	_playerPool.Free(player);
}


void ChattingServer::HandleLogin(int64 sessionId, CPacket* packet)
{
	printf("HandleLogin\n");
	Player* player = GetPlayer(sessionId);
	if (player == nullptr)
	{
		//TODO: log
		__debugbreak();
		return;
	}
	player->_lastRecvTime = timeGetTime();
	player->_bLogined = true;

	*packet >> player->accountNo;
	packet->GetData((char*)player->NickName, NICKNAME_LEN * sizeof(WCHAR));


	CPacket* resPacket = CPacket::Alloc();
	uint8 status = true;
	MP_SC_LOGIN(resPacket, player->accountNo, status);

	SendPacket_Unicast(player, resPacket);
	CPacket::Free(resPacket);
}

void ChattingServer::HandleHeartBeat(int64 sessionId, CPacket* packet)
{
	Player* player = GetPlayer(sessionId);
	if (player == nullptr)
	{
		//TODO: log
		__debugbreak();
		return;
	}

	player->_lastRecvTime = timeGetTime();
}

void ChattingServer::HandleMessage(int64 sessionId, CPacket* packet)
{
	printf("HandleMessage\n");
	Player* player = GetPlayer(sessionId);
	if (player == nullptr)
	{
		//TODO: log
		__debugbreak();
		return;
	}
	player->_lastRecvTime = timeGetTime();

	int64 accountNo;
	uint16 messageLen;
	*packet >> accountNo >> messageLen;
	//*packet >> messageLen;
	CPacket* resPacket = CPacket::Alloc();
	MP_SC_CHAT_MESSAGE(resPacket, accountNo, player->NickName, messageLen, packet);

	for (auto it = _playerMap.begin(); it != _playerMap.end(); ++it)
	{
		Player* player = (*it).second;
		SendPacket_Unicast(player, resPacket);
	}
	//MP_SC_CHAT_MESSAGE
	//TODO: 함수로 정리
	CPacket::Free(resPacket);
}





void ChattingServer::LogServerInfo()
{
	int64 totalAcceptValue = GetTotalAcceptValue();
	int64 acceptTPSValue = GetAcceptTPS();
	int64 sendMessageTPSValue = GetSendMessageTPS();
	int64 recvMessageTPSValue = GetRecvMessageTPS();
	int64 totalDisconnect = GetTotalDisconnect();
	//int64 playerReuseCount = _playerReuseCount;
	int64 sessionNum = GetSessionNum();
	int64 disconnectBySendQueueFull = GetDisconnectSessionNumBySendQueueFull();
	int64 jobQueueSize = _packetQueue.Size();
	int64 updateTps = GetUpdateTps();
	int processUserAllocMemory = _processUserAllocMemory;
	int networkSend = _networkSend;
	int networkRecv = _networkRecv;
	printf("Total Accept : %lld\n\
Accept TPS : % lld\n\
Send Message TPS : %lld\n\
Recv Message TPS : %lld\n\
session num :%lld\n\
total disconnect : %lld\n\
disconnect by sendqueue full : %lld\n\
jobQueueSize : %lld\n\
updateTps : %lld\n\
memory : %d\n\
send: %d\n\
recv: %d\n\
================\n\
", totalAcceptValue, acceptTPSValue, sendMessageTPSValue, recvMessageTPSValue, sessionNum,
 totalDisconnect, disconnectBySendQueueFull, jobQueueSize, updateTps, processUserAllocMemory, networkSend, networkRecv);
}

void ChattingServer::ObjectPoolLog()
{
	printf("packetqueue Object Pool Capcity : %lld\n", _packetQueue.GetObjectPoolCapacity());
	printf("playerMapsize : %lld\n", _playerMap.size());
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


bool ChattingServer::ActivateMonitorClient(const WCHAR* serverIp, uint16 port, uint32 concurrentThreadNum, uint32 workerThreadNum)
{
	_monitorClient = new MonitorClient();
	// iocp 생성
	bool bMonitorStartSucceed = _monitorClient->Start(serverIp, port, concurrentThreadNum, workerThreadNum);
	if (!bMonitorStartSucceed)
		return false;
	// 연결
	bool bConnectSucceed = _monitorClient->Connect();
	if (!bConnectSucceed)
		return false;

	int errorCode;
	_hMonitorSendThread = (HANDLE)_beginthreadex(NULL, 0, MonitorSendThreadStatic, this, 0, NULL);
	if (_hMonitorSendThread == NULL)
	{
		errorCode = WSAGetLastError();
		LOG(L"System", LogLevel::System, L"Create Monitor Send Thread Failed");
		__debugbreak();
	}

	_monitorActivated = true;
	return true;
}


unsigned int __stdcall ChattingServer::MonitorSendThread()
{
	/*uint16 sessionNumPerSector[50][50];
	PerformanceMonitorData performanceMonitorData;
	CpuUsage cpuUsage;
	CPacket* loginPacket = CPacket::Alloc();
	int serverNo = CHAT_SERVER_NO;
	MP_SS_MONITOR_LOGIN(loginPacket, serverNo);
	_monitorClient->SendPacket(loginPacket);
	CPacket::Free(loginPacket);*/

	Sleep(1000);
	while (!isNetworkStop())
	{
		//1초마다 한번씩 보내기 좀 더 늘릴까?
		Sleep(1000);


		//long long llTick;
		//time(&llTick);
		//int sendTick = (int)llTick;
		////채팅서버 cpu 사용률
		////채팅서버 메모리 사용 MByter
		////채팅서버 세션수
		////채팅서버 인증성공 사용자 수
		////채팅 서버 Update 쓰레드 초당 처리 횟수
		////채팅서버 패킷풀 사용량
		////채팅서버 update msg 풀 사용량


		//_performanceMonitor.Update(performanceMonitorData);
		//cpuUsage.UpdateCpuTime();

		//int processorCpuTotal = cpuUsage.ProcessorTotal();


		////PerformanceMonitorData'
		////server cpu
		////채팅서버 패킷 풀 사용량
		//int packetUseCount = (int)CPacket::GetUseCount();
		//CPacket* usecountpacket = CPacket::Alloc();
		//uint8 dataType = dfMONITOR_DATA_TYPE_CHAT_PACKET_POOL;
		//MP_SC_MONITOR_TOOL_DATA_UPDATE(usecountpacket, dataType, packetUseCount, sendTick);
		//_monitorClient->SendPacket(usecountpacket);
		//CPacket::Free(usecountpacket);

		//int processCpuTotal = cpuUsage.ProcessTotal();
		//CPacket* cpupacket = CPacket::Alloc();
		//dataType = dfMONITOR_DATA_TYPE_CHAT_SERVER_CPU;
		//MP_SC_MONITOR_TOOL_DATA_UPDATE(cpupacket, dataType, processCpuTotal, sendTick);
		//_monitorClient->SendPacket(cpupacket);
		//CPacket::Free(cpupacket);
		////server mem
		//int processUserAllocMemory = (int)performanceMonitorData.processUserAllocMemoryCounterVal.doubleValue / 1'000'000;
		//_processUserAllocMemory = processUserAllocMemory;
		//CPacket* mempacket = CPacket::Alloc();
		//dataType = dfMONITOR_DATA_TYPE_CHAT_SERVER_MEM;
		//MP_SC_MONITOR_TOOL_DATA_UPDATE(mempacket, dataType, processUserAllocMemory, sendTick);
		//_monitorClient->SendPacket(mempacket);
		//CPacket::Free(mempacket);

		//// 세션 수
		//int sessionNum = (int)GetSessionNum();
		//CPacket* sessionpacket = CPacket::Alloc();
		//dataType = dfMONITOR_DATA_TYPE_CHAT_SESSION;
		//MP_SC_MONITOR_TOOL_DATA_UPDATE(sessionpacket, dataType, sessionNum, sendTick);
		//_monitorClient->SendPacket(sessionpacket);
		//CPacket::Free(sessionpacket);

		////인증 성공 사용자 수
		//int playerNum = (int)_playerPool.GetUseCount();
		//CPacket* playerpacket = CPacket::Alloc();
		//dataType = dfMONITOR_DATA_TYPE_CHAT_PLAYER;
		//MP_SC_MONITOR_TOOL_DATA_UPDATE(playerpacket, dataType, playerNum, sendTick);
		//_monitorClient->SendPacket(playerpacket);
		//CPacket::Free(playerpacket);

		////채팅서버 update 스레드 초당 처리 횟수
		//int updateTps = (int)GetUpdateTps();
		//CPacket* tpspacket = CPacket::Alloc();
		//dataType = dfMONITOR_DATA_TYPE_CHAT_UPDATE_TPS;
		//MP_SC_MONITOR_TOOL_DATA_UPDATE(tpspacket, dataType, updateTps, sendTick);
		//_monitorClient->SendPacket(tpspacket);
		//CPacket::Free(tpspacket);



		//// 채팅서버 잡큐
		//int jobQueueSize = _packetQueue.Size();
		//CPacket* jobQueuePacket = CPacket::Alloc();
		//dataType = dfMONITOR_DATA_TYPE_CHAT_UPDATEMSG_POOL;
		//MP_SC_MONITOR_TOOL_DATA_UPDATE(jobQueuePacket, dataType, jobQueueSize, sendTick);
		//_monitorClient->SendPacket(jobQueuePacket);
		//CPacket::Free(jobQueuePacket);

		////네트워크 송신
		//int networkSend = (int)GetNetworkSendByteTps() / 1000;
		//_networkSend = networkSend;
		//CPacket* networkSendPacket = CPacket::Alloc();
		//dataType = dfMONITOR_DATA_TYPE_CHATTING_SERVER_NETWORK_SEND;
		//MP_SC_MONITOR_TOOL_DATA_UPDATE(networkSendPacket, dataType, networkSend, sendTick);
		//_monitorClient->SendPacket(networkSendPacket);
		//CPacket::Free(networkSendPacket);


		////네트워크 수신
		//int networkRecv = (int)GetNetworkRecvByteTps()/ 1000;
		//_networkRecv = networkRecv;
		//CPacket* networkRecvPacket = CPacket::Alloc();
		//dataType = dfMONITOR_DATA_TYPE_CHATTING_SERVER_NETWORK_RECV;
		//MP_SC_MONITOR_TOOL_DATA_UPDATE(networkRecvPacket, dataType, networkRecv, sendTick);
		//_monitorClient->SendPacket(networkRecvPacket);
		//CPacket::Free(networkRecvPacket);


	}

	return 0;
}

unsigned int __stdcall ChattingServer::MonitorThread()
{

		int tpsIndex = 0;
		while (!isNetworkStop())
		{
			Sleep(1000);

			_packetTps[tpsIndex % TPS_ARR_NUM] = _updateTps;
			_updateTps = 0;

			tpsIndex++;
		}
		return 0;
	return 0;
}



void ChattingServer::SendPacket_Unicast(Player* p, CPacket* packet)
{
	SendPacket(p->_sessionId, packet);
}





inline Player* ChattingServer::GetPlayer(int64 sessionId)
{
	auto playerIt = _playerMap.find(sessionId);
	if (playerIt != _playerMap.end())
	{
		return (*playerIt).second;
	}
	return nullptr;

}