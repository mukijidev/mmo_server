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

void LobbyFieldThread::FrameUpdate(float deltaTime)
{

}



void LobbyFieldThread::OnEnterThread(int64 sessionId, void* ptr)
{
	//TODO: map에 추가
	//TODO: 플레이어 생성
	Player* p = (Player*)ptr;
	p->SetField(this);

	auto result = _playerMap.insert({ sessionId, p });
	if (!result.second)
	{
		__debugbreak();
	}
	auto result2 = _fieldObjectMap.insert({ p->GetObjectId(), p });
	if (!result2.second)
	{
		__debugbreak();
	}

	p->StopMove();
	// 필드 이동 응답 보내고, 로그인쓰레드에서 fieldID 받긴하는데 어차피 처음엔 lobby니가
	CPacket* packet = CPacket::Alloc();
	uint8 status = true;
	uint16 fieldID = _gameThreadID;
	MP_SC_FIELD_MOVE(packet, status, fieldID);
	//TODO: send
	SendPacket_Unicast(p->_sessionId, packet);
	//printf("send field move\n");
	CPacket::Free(packet);

	// 내 캐릭터 소환 패킷 보내고
	int spawnX = rand() % GetMapXSize();
	int spawnY = rand() % GetMapYSize();
	for (;;)
	{
		Pos spawnPos;
		spawnPos.x = spawnX;
		spawnPos.y = spawnY;

		if (CheckValidPos(spawnPos))
			break;

		spawnX = rand() % GetMapXSize();
		spawnY = rand() % GetMapYSize();
	}

	spawnX = 8000 + rand() % 200;
	spawnY = 8000 + rand() % 200;
	//int spawnX = GetMapXSize() / 2 + rand() % 300;
	//int spawnY = GetMapYSize() / 2 + rand() % 300;

	CPacket* spawnCharacterPacket = CPacket::Alloc();

	FVector spawnLocation{ spawnX, spawnY,  PLAYER_Z_VALUE };
	FRotator spawnRotation{ 0, 0, 0 };

	p->Position = spawnLocation;
	p->Rotation = spawnRotation;

	p->_sectorYSize = _sectorYSize;
	p->_sectorXSize = _sectorXSize;
	p->_currentSector = &_sector[spawnY / _sectorYSize][spawnX / _sectorXSize];

	PlayerInfo myPlayerInfo = p->playerInfo;
	MP_SC_SPAWN_MY_CHARACTER(spawnCharacterPacket, myPlayerInfo, spawnLocation, spawnRotation);
	SendPacket_Unicast(p->_sessionId, spawnCharacterPacket);
	//printf("send spawn my character\n");
	CPacket::Free(spawnCharacterPacket);

	p->OnFieldChange();
}


void LobbyFieldThread::OnLeaveThread(int64 sessionId, bool disconnect)
{
	//TODO: 이 플레이어 despawn메시지 보내기
	auto it = _playerMap.find(sessionId);
	if (it == _playerMap.end())
	{
		LOG(L"GuardianFieldThread", LogLevel::Error, L"Cannot find sessionId : %lld, OnLeaveThread", sessionId);
		return;
	}
	Player* player = it->second;
	int64 characterNo = player->playerInfo.PlayerID;

	player->OnLeave();

	//맵에서 삭제
	int deletedNum = _playerMap.erase(sessionId);
	if (deletedNum == 0)
	{
		LOG(L"GuardianFieldThread", LogLevel::Error, L"Cannot find sessionId : %lld, OnLeaveThread", sessionId);
	}

	//섹터에서 삭제
	auto vectorIt = std::find(player->_currentSector->fieldObjectVector.begin(), player->_currentSector->fieldObjectVector.end(), (FieldObject*)player);
	if (vectorIt == player->_currentSector->fieldObjectVector.end())
	{
		__debugbreak();
	}
	player->_currentSector->fieldObjectVector.erase(vectorIt);


	//필드 오브젝트 맵에서 삭제
	int64 objectId = player->GetObjectId();
	size_t size = _fieldObjectMap.erase(objectId);
	if (size == 0)
	{
		__debugbreak();
	}

	//나간거면 free player
	if (disconnect)
	{
		FreePlayer(sessionId);
	}
	else
	{
		LOG(L"GuardianFieldThread", LogLevel::Error, L"no disconnect : %lld, OnLeaveThread", sessionId);
	}
}



