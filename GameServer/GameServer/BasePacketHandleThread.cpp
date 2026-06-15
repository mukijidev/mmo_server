#include "BasePacketHandleThread.h"
#include "SerializeBuffer.h"
#include "Packet.h"
#include "GameServer.h"

BasePacketHandleThread::BasePacketHandleThread(GameServer* gameServer, int threadId, int msPerFrame) : GameThread(threadId, msPerFrame)
{
	_gameServer = gameServer;
	SetGameServer((CNetServer*)gameServer);
}

void BasePacketHandleThread::RegisterPacketHandler(uint16 packetCode, PacketHandler handler)
{
	//이렇게 해서 교체까지
	_packetHandlerMap[packetCode] = handler;
}

int64 BasePacketHandleThread::GetPlayerSize()
{
	return _playerMap.size();
}

void BasePacketHandleThread::HandleRecvPacket(int64 sessionId, CPacket* packet)
{
	Player* player = nullptr;
	auto it = _playerMap.find(sessionId);
	if (it == _playerMap.end())
	{
		//TODO: Disconnect Player
		__debugbreak();
		DisconnectPlayer(sessionId);
		LOG(L"BasePacketHandleThread", LogLevel::Debug, L"Cannot find sessionId : %lld, HandleRecvPacket", sessionId);
		CPacket::Free(packet);
		return;
	}

	player = it->second;

	uint16 packetType;
	*packet >> packetType;
	auto handlerIt = _packetHandlerMap.find(packetType);
	if (handlerIt != _packetHandlerMap.end())
	{
		handlerIt->second(player, packet);
	}
	else
	{
		//TODO: Disonncet Player
		__debugbreak();
		DisconnectPlayer(sessionId);
		LOG(L"BasePacketHandleThread", LogLevel::Debug, L"Cannot find packetType : %lld, HandleRecvPacket", sessionId);
		CPacket::Free(packet);
		return;
	}

	CPacket::Free(packet);
}


void BasePacketHandleThread::DisconnectPlayer(int64 sessionId)
{
	_gameServer->Disconnect(sessionId);
}

Player* BasePacketHandleThread::AllocPlayer(int64 sessionId)
{
	return _gameServer->AllocPlayer(sessionId);
}

void BasePacketHandleThread::FreePlayer(int64 sessionId)
{
	_gameServer->FreePlayer(sessionId);
}

void BasePacketHandleThread::SendPacket(int64 sessionId, CPacket* packet)
{
	_gameServer->SendPacket(sessionId, packet);
}

