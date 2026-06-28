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
		uint16 sectorYLen, uint16 sectorXLen, uint16 sectorYSize, uint16 sectorXSize, uint8** map, uint8** coarseMap);

private:


private:
	void HandleCharacterAttack(Player* p, CPacket* packet);
	void UpdatePlayers(float deltaTime);

protected:
	//virtual void GetSpawnXY(int& outX, int& outY) override;s

public:
	// FieldPacketHandleThread을(를) 통해 상속됨
	void FrameUpdate(float deltaTime) override;
};

