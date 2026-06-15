#pragma once
#include "Type.h"
#include "GameThread.h"
#include <map>
#include <vector>
#include "ObjectPool.h"
#include "Monster.h"
#include <functional>
class CPacket;
class Player;
class GameServer;

class BasePacketHandleThread : public GameThread
{
	friend class FieldObject;

public:
	BasePacketHandleThread(GameServer* gameServer, int threadId, int msPerFrame);

protected:
	void SendPacket(int64 sessionId, CPacket* packet);
	// PacketHandler 
	using PacketHandler = std::function<void(Player*, CPacket*)>;
	void RegisterPacketHandler(uint16 packetCode, PacketHandler handler);

public:
	int64 GetPlayerSize() override;

protected:
	std::unordered_map<int64, Player*> _playerMap;
	Player* AllocPlayer(int64 sessionId);
	void FreePlayer(int64 sessionId);
	void DisconnectPlayer(int64 sessionId);

private:
	void HandleRecvPacket(int64 sessionId, CPacket* packet) override;
	std::map<uint16, PacketHandler> _packetHandlerMap;
	GameServer* _gameServer;
};

