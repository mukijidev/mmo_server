#pragma once
#include <map>
#include <vector>
#include "ObjectPool.h"
#include "FieldPacketHandleThread.h"

class Player;
class CPacket;
class GameServer;

class LobbyFieldThread : public FieldPacketHandleThread
{
public:
	LobbyFieldThread(GameServer* gameServer, int threadId, int msPerFrame,
		uint16 sectorYLen, uint16 sectorXLen, uint16 sectorYSize, uint16 sectorXSize, uint8** map);

private:
	//virtual void GameRun(float deltaTime) override;
	// GameThread을(를) 통해 상속됨
	void OnLeaveThread(int64 sessionId, bool disconnect) override;
	void OnEnterThread(int64 sessionId, void* ptr) override;

private:
	void HandleCharacterAttack(Player* p, CPacket* packet);
	void UpdatePlayers(float deltaTime);


public:
	// FieldPacketHandleThread을(를) 통해 상속됨
	void FrameUpdate(float deltaTime) override;
};

