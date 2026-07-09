#include "CLanServer.h"
#include <stdio.h>
#include "PacketHeader.h"
#include "SerializeBuffer.h"
#include "LockGuard.h"
#include <process.h>
#include "Log.h"
#include "Profiler.h"

CLanServer::CLanServer()
{
	//생성자에서 뭐할까?
}

CLanServer::~CLanServer()
{
	//소멸자에서 뭐할까?
}

bool CLanServer::Start(uint16 port, uint32 concurrentThreadNum, uint32 workerThreadNum, int nagle, int sendZeroCopy)
{
	LOG(L"System", LogLevel::System, L"Init Network Library Start");
	//wprintf(L"initnetwork start\n");
	_serverPort = port;
	_nagle = nagle;
	_sendZeroCopy = sendZeroCopy;

	int errorCode;
	// 세션 배열 초기화
	InitSessionArr();
	hReleaseEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (hReleaseEvent == NULL)
	{
		errorCode = WSAGetLastError();
		__debugbreak();
	}

	// 서버 iocp port
	_iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, concurrentThreadNum);
	if (_iocpHandle == NULL)
	{
		errorCode = WSAGetLastError();
		return false;
	}

	//worker thread
	HANDLE hWorkerThread;
	for (uint32 i = 0; i < workerThreadNum; i++)
	{
		hWorkerThread = (HANDLE)_beginthreadex(NULL, 0, WorkerThreadStatic, this, 0, NULL);
		if (hWorkerThread == NULL)
		{
			errorCode = WSAGetLastError();
			return false;
		}
		_threadList[_threadNumber++] = hWorkerThread;
	}

	//monitor thread
	HANDLE hMonitorThread;;
	hMonitorThread = (HANDLE)_beginthreadex(NULL, 0, MonitorThreadStatic, this, 0, NULL);
	if (hMonitorThread == NULL)
	{
		errorCode = WSAGetLastError();
		return false;
	}
	_threadList[_threadNumber++] = hMonitorThread;

	//release thread
	HANDLE hReleaseThread;
	hReleaseThread = (HANDLE)_beginthreadex(NULL, 0, ReleaseThreadStatic, this, 0, NULL);
	if (hReleaseThread == NULL)
	{
		errorCode = WSAGetLastError();
		return false;
	}
	_threadList[_threadNumber++] = hReleaseThread;


	//accept thread
	HANDLE hAcceptThread;
	hAcceptThread = (HANDLE)_beginthreadex(NULL, 0, AcceptThreadStatic, this, 0, NULL);
	if (hAcceptThread == NULL)
	{
		errorCode = WSAGetLastError();
		return false;
	}
	_threadList[_threadNumber++] = hAcceptThread;


	LOG(L"System", LogLevel::System, L"Init Network Library End");
	return true;
}

void CLanServer::Stop()
{
	LOG(L"System", LogLevel::System, L"Stop Network Library Start");
	_bStopNetwork = true; // accept thread, monitor thread, release thread
	SetEvent(hReleaseEvent);
	closesocket(_listenSocket); // accept thread
	PostQueuedCompletionStatus(_iocpHandle, 0, 0, nullptr); // worker thread
	WaitForMultipleObjects(_threadNumber, _threadList, true, INFINITE); // thread 기달리고
	ClearSesssionArr(); // session 배열 정리
	CloseHandle(_iocpHandle); // iocp handle 닫기
	LOG(L"System", LogLevel::System, L"Stop Network Library End");
}


void CLanServer::Disconnect(Session* session)
{
	session->_disconnectRequested = true;
	CancelIoEx((HANDLE)session->_socket, (LPOVERLAPPED)&session->_recvOverlapped);
	CancelIoEx((HANDLE)session->_socket, (LPOVERLAPPED)&session->_sendOverlapped);
}

void CLanServer::Disconnect(int64 sessionId)
{
	// TODO: session 찾아서
	// interlocked increment해서 내 세션으로 하고
	// send overlapp 끊고
	// recv overlapp 끊고
	// 더이상 send랑 recv 못하게하고
	if (sessionId < 0)
		return;

	Session* session = GetSession(sessionId);
	if (session == nullptr)
	{
		return;
	}

	session->_disconnectRequested = true;
	CancelIoEx((HANDLE)session->_socket, (LPOVERLAPPED)&session->_recvOverlapped);
	CancelIoEx((HANDLE)session->_socket, (LPOVERLAPPED)&session->_sendOverlapped);

	PutBackSession(session);
	return;
}



void CLanServer::SendPackets(int64 sessionId, std::vector<CPacket*>& packets)
{
	PRO_BEGIN(L"SendPackets");
	if (sessionId < 0)
	{
		PRO_END(L"SendPackets");
		return;
	}


	Session* session = GetSession(sessionId);
	if (session == nullptr)
	{
		PRO_END(L"SendPackets");

		return;
	}

	// 컨텐츠로부터 disconnect 요청 받은거였으면
	if (session->_disconnectRequested)
	{
		// 다시 집어넣고 return
		// disconnect요청이 이후에 왔을수도 있음  일단 여기서 한번 거르고
		PutBackSession(session);
		PRO_END(L"SendPackets");
		return;
	}


	// 패킷을 여러 세션에 보내기 위해 사용되는 packet refcount
	//int dataSize = sizeof(packet);
	int packetSize = packets.size();
	for (int i = 0; i < packetSize; i++)
	{
		CPacket*& packet = packets[i];

		packet->IncRefCount();
		packet->Encode(_packetKey);
		PRO_BEGIN(L"SendPacketEnqueue");
		session->_sendQueue.Enqueue(packet);
		PRO_END(L"SendPacketEnqueue");
	}

	InterlockedAdd64(&_processSendPacket, packetSize); // 로그용

	SendPost(session);
	PutBackSession(session);


	//PutBackSession(session);
	PRO_END(L"SendPackets");
	return;
}

//Enqueue랑 SendPost랑 다해야하나
void CLanServer::SendPacket(int64 sessionId, CPacket* packet)
{
	if (sessionId < 0)
		return;


	Session* session = GetSession(sessionId);
	if (session == nullptr)
	{
		return;
	}

	// 컨텐츠로부터 disconnect 요청 받은거였으면
	if (session->_disconnectRequested)
	{
		// 다시 집어넣고 return
		// disconnect요청이 이후에 왔을수도 있음  일단 여기서 한번 거르고
		PutBackSession(session);
		return;
	}

	InterlockedIncrement64(&_processSendPacket); // 로그용

	// 패킷을 여러 세션에 보내기 위해 사용되는 packet refcount
	packet->IncRefCount();

	//인코딩 하고 큐에 넣고
	packet->Encode(_packetKey);
	session->_sendQueue.Enqueue(packet);

	SendPost(session);
	PutBackSession(session);
	return;
}


unsigned __stdcall CLanServer::WorkerThread()
{
	int retVal;

	while (true)
	{

		DWORD cbTransferred = 0;
		SOCKET clientSock = NULL;

		OVERLAPPED* overlapped = nullptr;
		Session* s = nullptr;

		retVal = GetQueuedCompletionStatus(_iocpHandle, &cbTransferred,
			(PULONG_PTR)&s, &overlapped, INFINITE);


		if (s == nullptr)
		{
			if (retVal == 0)
			{
				int errorCode = WSAGetLastError();
				LOG(L"WorkerThread", LogLevel::Error, L"GQCS ret 0");
			}

			// 다른 워커쓰레드한테 전파
			PostQueuedCompletionStatus(_iocpHandle, 0, 0, 0);
			break;
		}

		if (overlapped == (OVERLAPPED*)0x12345678)
		{
			ReqWSASend(s);
			continue;
		}

		if (cbTransferred == 0)
		{
			DecIoCount(s);
			continue;
		}


		if (overlapped == &(s->_sendOverlapped))
		{
			int32 networkSend = (int32)cbTransferred + (((int32)cbTransferred) / 1460 + 1) * 40;
			InterlockedAdd(&_networkSend, networkSend);
			{
				// session이 wsasend한 큐
				s->ClearSendedQueue();
				InterlockedExchange8((CHAR*)(&s->_isSending), false);
				SendPost(s);

			}
		}

		else if (overlapped == &(s->_recvOverlapped))
		{
			int32 networkRecv = (int32)cbTransferred + (((int32)cbTransferred) / 1460 + 1) * 40;
			InterlockedAdd(&_networkRecv, networkRecv);

			int moveRecvRearVal = s->_recvQueue.MoveRear(cbTransferred);
			if (moveRecvRearVal != cbTransferred)
			{
				// ringbuffer에러
				__debugbreak();
			}

			bool bEnqueued = false;
			while (true)
			{
				// 패킷 헤더 만큼 못빼오는 경우
				if (s->_recvQueue.GetUseSize() < sizeof(NetHeader))
				{
					break;
				}

				// 헤더만큼 일단 Peek해오는데
				NetHeader header;
				int peekVal = s->_recvQueue.Peek((char*)&header, sizeof(NetHeader));
				if (peekVal != sizeof(NetHeader))
				{
					__debugbreak();
				}

				if (header._code != Data::clientPacketCode)
				{
					LOG(L"PacketCode", LogLevel::Error, L"packet code error, sessionId=%lld", s->_sessionId);
					Disconnect(s);
					break;
				}

				// len만큼 못빼오는 경우
				int dataSize = header._len;
				if (s->_recvQueue.GetUseSize() < sizeof(NetHeader) + dataSize)
				{
					break;
				}

				// header peek한것 읽은것으로 처리하고
				int moveHeaderVal = s->_recvQueue.MoveFront(sizeof(NetHeader));
				if (moveHeaderVal != sizeof(NetHeader))
				{
					__debugbreak();
				}

				// packet을 만들어서
				CPacket* packet = CPacket::Alloc();
				char* buf = packet->GetBufferPtr();
				// datasize만큼 긁어오고
				s->_recvQueue.Dequeue(buf, dataSize);
				packet->MoveWritePos(dataSize);
				//디코딩  ( checksum 실패 )
				bool bDecodeSucceed = packet->Decode(&header, _packetKey);
				if (!bDecodeSucceed)
				{
					//checksum 실패했을시
					//dec io count를 하고 다시 recvpost를 안함으로써 세션을 종료한다
					Disconnect(s);
					break;
				}
				// 패킷 컨텐츠로 넘기고
				s->_packetQueue.Enqueue(packet);
				bEnqueued = true;
				InterlockedIncrement64(&_processRecvPacket);
			}

			//if (bEnqueued)
			//{
			//	s->_gameThread->SetUpdateEvent();
			//}


			if (!s->_disconnectRequested)
			{
				//checksum 실패했을시
				//dec io count를 하고 다시 recvpost를 안함으로써 세션을 종료한다
				RecvPost(s);
			}
		}
		else {
			__debugbreak();
		}

		// 완료통지시 iocount를 1 감소
		// 다시 recv Post, 어느 순간이든 session이 끊기기 전까지는 recv post로 iocount를 1이상 유지한다
		DecIoCount(s);
	}

	return 0;
}


unsigned __stdcall CLanServer::AcceptThread()
{
	LOG(L"System", LogLevel::System, L"Accept Thread Start");
	// 서버 ip port
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(_serverPort);

	// 리슨 소켓 만들고
	_listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_listenSocket == INVALID_SOCKET)
	{
		int error = WSAGetLastError();
		return 0;
	}

	// linger fin 보내기
	LINGER optval;
	optval.l_onoff = 1;
	optval.l_linger = 0;
	int optRet = setsockopt(_listenSocket, SOL_SOCKET, SO_LINGER, (char*)&optval, sizeof(optval));
	if (optRet == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		__debugbreak();
		return false;
	}

	// zero copy 옵션 추가했으면
	if (_sendZeroCopy == 1)
	{
		int sendZeroCopy = 0; // 송신 버퍼 크기를 0으로 설정
		int retVal = setsockopt(_listenSocket, SOL_SOCKET, SO_SNDBUF, (char*)&sendZeroCopy, sizeof(sendZeroCopy));
		if (retVal == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			wprintf(L"setsockopt sndbuf failed : %d\n", error);
			__debugbreak();
		}
	}


	// bind하고
	int bindRetVal;
	bindRetVal = bind(_listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (bindRetVal == SOCKET_ERROR)
	{
		//bind failed
		wprintf(L"bind failed\n");
		return false;
	}

	int listenRetVal;
	listenRetVal = listen(_listenSocket, SOMAXCONN_HINT(10000));
	if (listenRetVal == SOCKET_ERROR)
	{
		// listen failed
		wprintf(L"listend failed\n");
		return false;
	}


	while (!_bStopNetwork)
	{
		int addrLen;
		//clientSocket;

		SOCKADDR_IN clientAddr;

		//accept
		addrLen = sizeof(clientAddr);
		SOCKET clientSocket = accept(_listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
		if (clientSocket == INVALID_SOCKET)
		{
			int error = WSAGetLastError();
			/*
			*  listen socket 닫으면 생기는에러 10004

				WSAEINTR
				10004
				WSACancelBlockingCall
			*/
			if (_bStopNetwork)
			{
				LOG(L"System", LogLevel::System, L"Accept Thread End By Keyboard Input");
			}
			else {
				LOG(L"System", LogLevel::System, L"Accept Error");
			}
			break;
		}

		_totalAccept++;
		_acceptPacket++;

		bool bAcceptSuccess = AcceptProcess(clientSocket, &clientAddr);
		if (!bAcceptSuccess)
		{
			/*__debugbreak();*/
		}
	}
	LOG(L"System", LogLevel::System, L"Accept Thread End");
	return 0;
}



unsigned __stdcall CLanServer::MonitorThread()
{
	int tpsIndex = 0;
	while (!_bStopNetwork)
	{
		Sleep(1000);

		_monitoredSendTPS[tpsIndex % TPS_ARR_NUM] = _processSendPacket;
		_processSendPacket = 0;

		_monitoredRecvTPS[tpsIndex % TPS_ARR_NUM] = _processRecvPacket;
		_processRecvPacket = 0;

		_monitoredAcceptTPS[tpsIndex % TPS_ARR_NUM] = _acceptPacket;
		_acceptPacket = 0;

		_monitoredNetworkByteSendTPS[tpsIndex % TPS_ARR_NUM] = _networkSend;
		_networkSend = 0;

		_monitoredNetworkByteRecvTPS[tpsIndex % TPS_ARR_NUM] = _networkRecv;
		_networkRecv = 0;



		tpsIndex++;
	}
	return 0;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
///							Release Thread 													     //
///																								 //
///  만든 이유:																					 //
///  Contents에서 contents에서 필요한 lock을 잡고 sendpacket을 하는데 session이 disconenct되서		 //
///  바로 on disconnect까지 타버리면 같은 쓰레드에서 on disconnect가 처리되는데						 //
///	 on disconnect에서 같은 lock을 또 잡아버리면 dead lock 발생									 //
///																								 //
///  작은 contents만 구현하면 어떻게든 contents에서 처리가 가능 하겠지만						     //
///  이것은 library 코드 구조를 알아야 가능한거고												 	 //
///  애초에 library 코드를 신경안쓰고 컨텐츠가 구현되야하는게 맞는것이니								 //
///																								 //
///  OnDisconnect호출될 경우에, release thread에 enqueue하고 return하고 contents에서는				 //
///  RelesaeThread가 OnDisconnect를 비동기로 호출한다										         //
/// ///////////////////////////////////////////////////////////////////////////////////////////////
unsigned __stdcall CLanServer::ReleaseThread()
{
	int error;
	while (!_bStopNetwork)
	{
		DWORD result = WaitForSingleObject(hReleaseEvent, INFINITE);
		if (result == WAIT_FAILED)
		{
			error = WSAGetLastError();
			__debugbreak();
		}

		{
			while (true)
			{
				Session* session = nullptr;
				bool dequeueSucceed = _releaseSessionQueue.Dequeue(session);
				if (!dequeueSucceed)
				{
					break;
				}

				if (session->_gameThread == nullptr)
				{
					//없는거 말이안됨
					__debugbreak();
				}

				CPacket* packet;
				while (true)
				{
					bool bDequeueSucceed = session->_packetQueue.Dequeue(packet);
					if (!bDequeueSucceed)
					{
						break;
					}
					CPacket::Free(packet);
				}

				session->_gameThread->LeaveSession(session->_sessionId, true);
				//OnDisconnect(session->_sessionId);
				ReleaseSession(session);

				printf("release session\n");
			}
		}
	}
	return 0;
}

bool CLanServer::SendPost(Session* session)
{
	IncIoCount(session);
	int useSize = session->_sendQueue.Size();
	if (useSize == 0)
	{
		DecIoCount(session);
		return false;
	}

	if (_InterlockedExchange8((char*)(&session->_isSending), true) == true)
	{
		DecIoCount(session);
		return false;
	}

	if (session->_sendQueue.Size() == 0)
	{
		InterlockedExchange8((char*)(&session->_isSending), false);
		DecIoCount(session);
		return false;
	}

	PostQueuedCompletionStatus(_iocpHandle, 0, ULONG_PTR(session), (LPOVERLAPPED)0x12345678);
	return true;
}


//수신 요청
bool CLanServer::RecvPost(Session* session)
{
	IncIoCount(session);

	//WriteLock writeLock(&session->_sessionLock);
	int directEnqueueSize = session->_recvQueue.GetDirectEnqueueSize();
	// 수신버퍼가 꽉차는건 문제, 받으면 바로 처리해야하니까
	// 근데 서버 느리면 비동기 i/o에서는 꽉찰 수 도 있는건가

	if (directEnqueueSize == 0)
	{
		__debugbreak();
	}
	char* rearPtr = session->_recvQueue.GetRearPtr();


	int wsabufNum = 1;

	// bufferptr
	// [] [] [front ptr] [] [] [] [] [rearptr] [] [] [] [] []
	// rearptr부터 뒤에까지
	// 앞에부터 frontptr까지

	WSABUF wsabufArr[2];
	//WSABUF wsabufRear;
	wsabufArr[0].buf = rearPtr;
	wsabufArr[0].len = directEnqueueSize;

	int canReceiveSize = session->_recvQueue.GetFreeSize();
	if (canReceiveSize > directEnqueueSize)
	{
		wsabufNum = 2;
		char* bufferPtr = session->_recvQueue.GetBufferPtr();

		wsabufArr[1].buf = bufferPtr;
		wsabufArr[1].len = canReceiveSize - directEnqueueSize;
	}

	//printf("recv post receiveSize : %d\n", canReceiveSize);
	//DWORD recvBytes;
	DWORD flags;
	flags = 0;

	// recv overlapp
	memset(&(session->_recvOverlapped), 0, sizeof(session->_recvOverlapped));

	//비동기로 요청 했으면
	if (session->_disconnectRequested)
	{
		DecIoCount(session);
		return false;
	}

	int retVal = WSARecv(session->_socket, wsabufArr, wsabufNum, nullptr, &flags, &session->_recvOverlapped, NULL);
	if (retVal == SOCKET_ERROR)
	{
		int recvError = WSAGetLastError();
		if (recvError != ERROR_IO_PENDING)
		{
			//CancelIoEx((HANDLE)session->_socket, (LPOVERLAPPED)&session->_sendOverlapped);
			//TODO: 로그찍기
			if (recvError != WSAECONNRESET && recvError != WSAECONNABORTED)
			{
				LOG(L"Disconnect", LogLevel::Error, L"WSA Recv error : %d, errorCode :%d", session->_sessionId, recvError);
			}
			/*	if (recvError == WSAECONNRESET)
				{
					InterlockedIncrement64(&_recvConnResetTotal);
					LOG(L"Disconnect", LogLevel::Debug, L"Recv WSAECONNRESET : %d", session->_sessionId);
				}*/

			DecIoCount(session);
			return false;
		}
		else {
			//비동기로 요청 했으면
			if (session->_disconnectRequested)
			{
				CancelIoEx((HANDLE)session->_socket, (LPOVERLAPPED)&session->_recvOverlapped);
				//DecIoCount(session);
				return false;
			}
		}
		// ERROR IO PENDING이면 정상
	}
	return true;
}

bool CLanServer::AcceptProcess(SOCKET socket, SOCKADDR_IN* clientAddr)
{
	// 세션 생성하고
	Session* session = CreateSession(socket, clientAddr);
	if (session == nullptr)
	{
		return false;
	}
	/*if (session->_ioFlag._ioCount != 1)
	{
		__debugbreak();
	}*/

	// iocp에 등록하고
	HANDLE iocpHandle = CreateIoCompletionPort((HANDLE)socket, _iocpHandle, (ULONG_PTR)session, 0);
	if (iocpHandle == NULL)
	{
		//error code 6 이미 연결끊긴 소켓 등록하려는 것
		int errorCode = WSAGetLastError();

		LOG(L"Accept", LogLevel::Error, L"connect process create io completion port failed : %d\n", errorCode);
		return false;
	}

	// 컨텐츠 accept 처리
	//OnAccept(session->_sessionId);
	session->_gameThread = _defaultGameThread;
	_defaultGameThread->EnterSession(session->_sessionId, nullptr);

	//SessionLog sessionLog = { LogCode::Accpet, (uint32)__LINE__ };
	//LOG_SESSION(session, sessionLog);
	//
	//recv post 걸고
	RecvPost(session);

	// iocount 1 감소시킴 , session은 시작됐을댸 iocount 1이기 떄문에 recvpost건 순간 2이고, 여기서 감소시켜야 1임
	bool bDisconnected = DecIoCount(session);
	if (bDisconnected)
	{
		return false;
	}

	return true;
}


Session* CLanServer::CreateSession(SOCKET socket, SOCKADDR_IN* clientAddr)
{
	// 1씩 증가하는 sessionid
	static int64 GenerateSessionId = 0;
	int64 id = ++GenerateSessionId;
	uint16 sessionIndex = 0;

	//빈 index 찾아서
	bool bPopSucceed = _emptySessionIndex.Pop(sessionIndex);
	if (!bPopSucceed)
	{
		// TODO: 없으면 연결안받거나
		// 늘리거나
		// 지금은 Disconnect를 엄청해서 서버에서
		// 성능 안좋을 떄 실패할수도있음

		//__debugbreak();
		LOG(L"Accept", LogLevel::Error, L"!bPopSucceed from empty session index");
		return nullptr;
	}
	else {
		InterlockedIncrement64(&_sessionNum);
	}

	//세션 id 8 byte중 앞에 2byte sessionInde로 스고
	uint16* ptr = (uint16*)&id;
	ptr += 3;
	*ptr = sessionIndex;


	Session* session = nullptr;
	{
		session = &_sessionArr[sessionIndex];
	}

	session->Init(id, socket, clientAddr);
	return session;
}


// close socket을 1회하기 위해서
bool CLanServer::ReleaseSession(Session* session)
{
	uint64 sessionId = session->_sessionId;
	uint16 sessionIndex = GetSessionIndex(sessionId);

	int closeSocketError;
	int closeSocketErrorCode;
	{
		//WriteLock sessionLock(&session->_sessionLock);
		closeSocketError = closesocket(session->_socket);
		session->_socket = INVALID_SOCKET;
		if (closeSocketError != 0)
		{
			closeSocketErrorCode = WSAGetLastError();
			__debugbreak();
		}
	}


	session->ClearSendQueue();
	session->ClearSendedQueue();

	_emptySessionIndex.Push(sessionIndex);
	InterlockedDecrement64(&_sessionNum);
	InterlockedIncrement64(&_totalDisconnet);
	return true;
}

inline bool CLanServer::TryReleaseSession(Session* session)
{
	int64 exchangeValue = 0x0000'0001'0000'0001;

	if (InterlockedCompareExchange64((int64*)(&session->_ioFlag), exchangeValue, 0) != 0)
	{
		return false;
	}

	_releaseSessionQueue.Enqueue(session);
	SetEvent(hReleaseEvent);
	return true;
}


//이정도도 inline이 되나
// close socket을 1회하기 위해서
inline bool CLanServer::TryReleaseSession(Session* session, int line)
{
	// 목표, 변경할값, 예상값
	// 예상값 ioFlag = 0, ioCount = 0;
	// 변경할값 ioFlag = 1, ioCount = 0;
	int64 exchangeValue = 0x0000'0001'0000'0001;
	if (InterlockedCompareExchange64((int64*)(&session->_ioFlag), exchangeValue, 0) != 0)
	{
		//ioFlag가 0이 아니었으면
		// 아직 사용중인 세션임: relase 다음에
		return false;
	}

	//디버깅용
	//if (line == CHECK_RELEASE)
	//{
	//	InterlockedIncrement64(&_releaseFlageDisconnect);
	//}
	//LOG(L"Disconnect", LogLevel::Debug, L"ReleaseSession session Id: sessiond Id :%d, line :  %d", session->_sessionId, line);
	//SessionLog sessionLog = { LogCode::Disconnect, (uint32)line };
	//LOG_SESSION(session, sessionLog);


	// release thread에 enqueue한다
	_releaseSessionQueue.Enqueue(session);
	SetEvent(hReleaseEvent);
	return true;
}

//inline 처리 됐고
inline uint16 CLanServer::GetSessionIndex(int64 sessionId)
{
	//uint16* ptr = (uint16*)&sessionId;
	/*ptr += 3;
	return *ptr;*/
	return *((uint16*)&sessionId + 3);
}


//inline 됐고
inline void CLanServer::IncIoCount(Session* session)
{
	InterlockedIncrement(&session->_ioFlag._ioCount);
	//SessionLog sessionLog = { LogCode::IncIo, after };
	//LOG_SESSION(session, sessionLog);
}


/// <summary>
/// 
/// </summary>
/// <param name="session"></param>
/// <returns> return true when disconnected</returns>
inline bool CLanServer::DecIoCount(Session* session)
{
	if (InterlockedDecrement(&session->_ioFlag._ioCount) == 0)
	{
		TryReleaseSession(session);
		return true;
	}
	return false;

}

inline Session* CLanServer::GetSession(int64 sessionId)
{
	//Release랑 lock없이 동기화 하기 위해
	// ioCount를 증가시킨순간 release는 못하고 이 쓰레드 이함수의 세션임

	// io count가 1임
	// io count가 1이면 release는 못들어옴
	// 내가 그냥 ioCount 증가시켰음
	// 근데 relaseFlag 이미 true임
	// 그럼 이세션은 release된 세션이거나 release 중인 세션이고
	// 내가 보내려했던 세션이든. 다른 세션이든
	// 그냥 decremnet io count하고 나가면됨

	// release중인 세션이었다 -> iocount 감소시키고 나가면됨
	// release된 세션이었따 -> iocount 감소시키고 나가면됨
	// release되고 재사용 되었따 -> iocount 감소시키고 나가면됨
		//uint16 sessionIndex = 
		/*if (sessionIndex < 0 || sessionIndex > MAX_SESSION_NUM)
			return nullptr;*/
	Session* s = &_sessionArr[GetSessionIndex(sessionId)];

	IncIoCount(s);
	//이미 끊긴상태면
	if (s->_ioFlag._releaseFlag == 1 || sessionId != s->_sessionId)
	{
		DecIoCount(s);
		return nullptr;
	}
	return s;
	//return s;
	////int64 findedSessionId = s->_sessionId;
	//if (sessionId != s->_sessionId)
	//{
	//	DecIoCount(s);
	//	return nullptr;
	//}

	//return s;
}

inline void CLanServer::PutBackSession(Session* session)
{
	DecIoCount(session);
}

void CLanServer::ReqWSASend(Session* session)
{
	int retVal;
	int checkedQueueSize = session->_sendQueue.Size();
	// 바라본 size만큼 dequeue를 못할수 있는데

	/*if (useSize % 8 != 0)
	{
		__debugbreak();
	}*/

	memset(&(session->_sendOverlapped), 0, sizeof(session->_sendOverlapped));

	if (checkedQueueSize > MAX_PACKET_NUM_IN_SEND_QUEUE)
	{
		LOG(L"Disconnect", LogLevel::Error, L"sendqueue full session Id : %lld", session->_sessionId);
		InterlockedIncrement64(&_disconnectSessionNumBySendQueueFull);
		DecIoCount(session);
		Disconnect(session);
		return;
	}

	WSABUF wsaSendBuf[MAX_PACKET_NUM_IN_SEND_QUEUE];
	int dequeuedSize = 0;
	for (int i = 0; i < checkedQueueSize; i++)
	{

		CPacket* ptr = nullptr;
		bool bDequeueSucceed = session->_sendQueue.Dequeue(ptr);
		if (!bDequeueSucceed)
		{
			//실패했으면
			break;
		}
		wsaSendBuf[dequeuedSize].buf = ptr->GetBufferPtr();
		wsaSendBuf[dequeuedSize].len = ptr->GetDataSize();
		dequeuedSize++;


		session->_sendedQueue.push(ptr);
	}

	int wsabufNum = dequeuedSize;

	if (session->_disconnectRequested)
	{
		DecIoCount(session);
		return;
	}


	retVal = WSASend(session->_socket, wsaSendBuf, wsabufNum, nullptr, 0, &session->_sendOverlapped, NULL);

	if (retVal == SOCKET_ERROR)
	{
		int sendError = WSAGetLastError();
		if (sendError != WSA_IO_PENDING)
		{
			//CancelIoEx((HANDLE)session->_socket, (LPOVERLAPPED)&session->_recvOverlapped);

			// TODO: 로그 찍기
			if (sendError != WSAECONNRESET && sendError != WSAECONNABORTED)
			{
				LOG(L"Disconnect", LogLevel::Error, L"WSA SendError: %d, errorCode :%d", session->_sessionId, sendError);
			}

			//if (sendError == WSAECONNRESET)
			//{
			//	LOG(L"Disconnect", LogLevel::Debug, L"Send WSAECONNRESET : %d", session->_sessionId);
			//}
			DecIoCount(session);
			return;

		}
		else
		{
			//비동기로 요청 했으면
			if (session->_disconnectRequested)
			{
				CancelIoEx((HANDLE)session->_socket, (LPOVERLAPPED)&session->_sendOverlapped);
				//DecIoCount(session);
				return;

			}
			//DecIoCount(session);  완료통지는 들어간거니까 완료통지에서 DecIoCount해서 여기서 하면안됨
			return;
		}
	}
}


void CLanServer::RegisterGameThread(GameThread* gameThread)
{

}

void CLanServer::RegisterDefaultGameThread(GameThread* gameThread)
{
	_defaultGameThread = gameThread;
}



int64 CLanServer::GetSessionId(int64 sessionId)
{
	//6 byte
	// 0x0000'FFFF'FFFF'FFFF
	int64 demaskValue = 0x0000'FFFF'FFFF'FFFF;
	return sessionId & demaskValue;
}


void CLanServer::InitSessionArr()
{
	//memset(_sessionArr, 0, sizeof(_sessionArr));
	//InitializeCriticalSection(&_sessionIndexLock);
	for (int i = MAX_SESSION_NUM - 1; i >= 0; i--)
	{
		InitializeSRWLock(&_sessionArr[i]._lock);
		_emptySessionIndex.Push(i);
	}
}


void CLanServer::ClearSesssionArr()
{
	// packet queue에 남은거 반납
	// close socket
	for (int i = 0; i < MAX_SESSION_NUM; i++)
	{
		Session* session = &_sessionArr[i];
		if (session->_socket == INVALID_SOCKET)
			continue;

		closesocket(session->_socket);
		session->_socket = INVALID_SOCKET;

		session->ClearSendedQueue();
		session->ClearSendQueue();
		session->ClearPacketQueue();
	}
}