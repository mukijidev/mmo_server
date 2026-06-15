#include "Session.h"
#include "SerializeBuffer.h"

//void LogSession(Session* session, SessionLog log)
//{
//	int32 logindex = (InterlockedIncrement(&session->_logIndex)) % MAX_SESSION_LOG;
//	session->_logArr[logindex] = log;
//}

void Session::ClearSendedQueue()
{
	int size = _sendedQueue.size();
	for (int i = 0 ; i < size; i++)
	{
		CPacket* packet = _sendedQueue.front();
		CPacket::Free(packet);
		_sendedQueue.pop();
	}
}

void Session::Init(int64 sessionId, SOCKET socket, SOCKADDR_IN* clientAddr)
{
	_sessionId = sessionId;
	_socket = socket;
	int size = _sendQueue.Size();
	for (int i = 0; i < size; i++)
	{
		CPacket* packet = nullptr;
		bool bDequeueSucceed = _sendQueue.Dequeue(packet);
		if (!bDequeueSucceed)
		{
			__debugbreak();
		}
		CPacket::Free(packet);
	}

	ClearSendedQueue();
	if (_sendQueue.Size() != 0)
	{
		__debugbreak();
	}


	_recvQueue.ClearBuffer();
	//session->_sendQueue.ClearBuffer();
	_ioFlag._releaseFlag = 0;
	/*session->_ioCount = 0;*/
	_isSending = false;
	_disconnectRequested = false;
	//session->_sendPacketNum = 0;
	memset(&_recvOverlapped, 0, sizeof(_recvOverlapped));
	memset(&_sendOverlapped, 0, sizeof(_sendOverlapped));
	memset(_ip, 0, sizeof(_ip));

	//ip 및 port 저장
	//WCHAR str[INET_ADDRSTRLEN];//
	InetNtop(AF_INET, &((*clientAddr).sin_addr), _ip, INET_ADDRSTRLEN);
	_port = ntohs(clientAddr->sin_port);


}

void Session::Init(int64 sessionId, SOCKET socket)
{
	_sessionId = sessionId;
	_socket = socket;

	int size = _sendQueue.Size();
	for (int i = 0; i < size; i++)
	{
		CPacket* packet = nullptr;
		bool bDequeueSucceed = _sendQueue.Dequeue(packet);
		if (!bDequeueSucceed)
		{
			__debugbreak();
		}
		CPacket::Free(packet);
	}
	
	ClearSendedQueue();

	if (_sendQueue.Size() != 0)
	{
		__debugbreak();
	}

	_recvQueue.ClearBuffer();
	//session->_sendQueue.ClearBuffer();
	_ioFlag._releaseFlag = 0;
	/*session->_ioCount = 0;*/
	_isSending = false;
	_disconnectRequested = false;
	//session->_sendPacketNum = 0;
	memset(&_recvOverlapped, 0, sizeof(_recvOverlapped));
	memset(&_sendOverlapped, 0, sizeof(_sendOverlapped));
	memset(_ip, 0, sizeof(_ip));

	//ip 및 port 저장
	//WCHAR str[INET_ADDRSTRLEN];//
}