#include "GuardianFieldThread.h"
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
#include "Sector.h"



using namespace std;

GuardianFieldThread::GuardianFieldThread(GameServer* gameServer,int threadId, int msPerFrame,
	uint16 sectorYLen, uint16 sectorXLen, uint16 sectorYSize, uint16 sectorXSize, uint8** map)
	: FieldPacketHandleThread(gameServer, threadId, msPerFrame, sectorYLen, sectorXLen, sectorYSize, sectorXSize, map)
{

}

void GuardianFieldThread::FrameUpdate(float deltaTime)
{
	//TODO: 몬스터 갯수 확인하기
	// 몬스터 없으면 Spawn 하고
	int currentMonsterSize = _monsterMap.size();
	if (currentMonsterSize < _maxMonsterNum)
	{
		SpawnMonster();
		//printf("Spawn Monster\n");
	}

	for (auto it = _monsterMap.begin(); it != _monsterMap.end();)
	{
		MonsterState state = (*it).second->GetState();
		if (state == MonsterState::MS_DEATH)
		{

			ReturnFieldObject((*it).first);
			it = _monsterMap.erase(it);
		}
		else
		{
			it++;
		}
	}
}

void GuardianFieldThread::SpawnMonster()
{
	uint32 mapXSize = GetMapXSize();
	uint32 mapYSize = GetMapYSize();
	int spawnX  = rand() % mapXSize;
	int spawnY = rand() % mapYSize;

	if(CheckValidPos({ spawnY, spawnX }) == false)
	{
		return;
	}

	FVector randomLocation{ spawnX, spawnY, 88.1 };
	randomLocation.X = std::clamp(randomLocation.X, double(100), double(mapXSize - 100));
	randomLocation.Y = std::clamp(randomLocation.Y, double(100), double(mapYSize - 100));

	FRotator spawnRotation = { 0, 0, 0 };


	Monster* monster = AllocMonster(MONSTER_TYPE_GUARDIAN);
	if (monster == nullptr)
		return;

	monster->_position = randomLocation;
	monster->_rotation = spawnRotation;


	monster->_sectorXSize = _sectorXSize;
	monster->_sectorYSize = _sectorYSize;


	monster->_currentSector = &_sector[spawnY / _sectorYSize][spawnX / _sectorXSize];
	monster->_currentSector->fieldObjectVector.push_back(monster);
	monster->OnSpawn();
}



