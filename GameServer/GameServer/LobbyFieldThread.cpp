#include "LobbyFieldThread.h"
#include "SerializeBuffer.h"
#include "Profiler.h"
#include <process.h>
#include "Log.h"
#include "Packet.h"
#include "GameData.h"
#include <algorithm>
#include "PacketMaker.h"
#include "Monster.h"
#include "Player.h"
#include "GameServer.h"
#include "Sector.h"`


using namespace std;

LobbyFieldThread::LobbyFieldThread(GameServer* gameServer, int threadId, int msPerFrame,
	uint16 sectorYLen, uint16 sectorXLen, uint16 sectorYSize, uint16 sectorXSize, uint8** map, uint8** coarseMap) : FieldPacketHandleThread(gameServer, threadId, msPerFrame, sectorYLen, sectorXLen, sectorYSize, sectorXSize, map, coarseMap)
{
	RegisterPacketHandler(PACKET_CS_GAME_REQ_CHARACTER_ATTACK, [this](Player* p, CPacket* packet) { HandleCharacterAttack(p, packet); });
}


void LobbyFieldThread::HandleCharacterAttack(Player* p, CPacket* packet)
{
	//브로드 캐스팅
	//TODO: 서버에서 검증하기

	int32 attackerType;
	int64 attackerID;
	int32 targetType;
	int64 targetID;
	
	*packet >> attackerType >> attackerID >> targetType >> targetID;
}

//void LobbyFieldThread::GameRun(float deltaTime)
//{
//	UpdatePlayers(deltaTime);
//
//	// Lobby Thread에는 몬스터 없고
//	//UpdateMonsters(deltaTime);
//}

void LobbyFieldThread::UpdatePlayers(float deltaTime)
{
	for (auto it = _playerMap.begin(); it != _playerMap.end(); it++)
	{
		Player* player = it->second;
		player->Update(deltaTime);
	}
}

//void LobbyFieldThread::GetSpawnXY(int& outX, int& outY)
//{
//	outX = 8000 + rand() % 200;
//	outY = 8000 + rand() % 200;
//}

void LobbyFieldThread::FrameUpdate(float deltaTime)
{

}



