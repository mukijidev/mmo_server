#pragma once
#include "Type.h"
#include <unordered_map>
#include "FieldObject.h"
#include "JumpPointSearch.h"
#include <vector>

class Sector;
class CPacket;
class Player;
class BasePacketHandleThread;

//200 이상이면 길찾기 요청
//200 이하면 길찾기 요청안함

#define REQUEST_FIND_PATH_THRESHOLD 200.0
#define MAX_ROTATION_DIFFERENCE 30.0
#define MONSTER_REQUEST_PATH_MAX_FAIL_COUNT 5

enum class MonsterState {
	MS_IDLE,
	MS_MOVING,
	MS_ATTACKING,
	MS_CHASING,
	MS_DEATH,
};


// 이거 몬스터 override를 해서 비선공형 선공형을 나눠야할까?
// 아니면 MonsterType을 만들어서 비선공형 선공형을 나눠야할까?
class Monster : public FieldObject
{
	friend class GuardianFieldThread;
	friend class LobbyFieldThread;
	friend class SpiderFieldThread;
	friend class FieldPacketHandleThread;

public:
	//Getter
	MonsterInfo GetMonsterInfo() { return _monsterInfo; };
	FVector GetPosition() { return _position; };
	FRotator GetRotation() { return _rotation; };
	FVector GetDestination() { return _destination; };

public:
	Monster(FieldPacketHandleThread* field, uint16 objectType, uint16 monsterType);
	Monster(FieldPacketHandleThread* field, uint16 objectType, uint16 monsterType, FVector spawnPosition);
	void Init(uint16 monsterType);
	MonsterState GetState() { return _state; };
	void Update(float deltaTime);
	void SetDestination(FVector destination);
	//void HandleAsyncFindPath();
	/// <summary>
	/// 
	/// </summary>
	/// <param name="damage"></param>
	/// <param name="attacker"></param>
	/// <returns>return true when death</returns>
	bool TakeDamage(int damage, Player* attacker);
	bool MoveToDestination(float deltaTime);
	bool MoveToPlayer(float deltaTime);
	void AttackPlayer(float deltaTime);
	void ChasePlayer(float deltaTime);
	bool SetRandomDestination();
	float GetDistanceToPlayer(Player* targetPlayer);
	void OnSpawn();
	bool IsMoving() { return _state == MonsterState::MS_CHASING && !_path.empty(); }


//private:
//	void SendIdlePacket();
//	void SendAttackPacket();
//	void SendDestinationPacket();
	
	//Player가 나가거나 죽을떄 해당 player를 target으로 하고있던애를 idle 상태로 만들어야함
	void SetTargetPlayerEmpty();

private:
	MonsterInfo _monsterInfo;
	FVector _position; // 현재 위치
	FRotator _rotation{ 0,0,0 }; // 현재 방향

	FVector _destination; // 목적지
	float _speed; // 이동속도
	MonsterState _state;
	
	Player* _targetPlayer; // 공격중인 플레이어
	float _attackRange; // 공격 범위
	float _attackCooldown; // 공격 쿨타임
	float _attackTimer = 0; // 공격 후 타이머 리셋하기
	float _aggroRange; // 어그로 범위
	float _idleTime;// 일정시간 동안 대기할 시간
	float _chaseTime;// 플레이어를 추적하는 시간
	float _maxChaseTime; // 최대 추적시간

	//
	int _damage = 5;	
	float _defaultIdleTime = 10;

private:
	void ProcessSectorChange(Sector* newSector);
	void AddSector(Sector* newSEctor);
	void RemoveSector(Sector* newSector);
	void SendMovePacket();
//
public:
	//std::vector<Pos> _path;
	//uint16 _pathIndex = 0;
//	bool bRequestPath = false;

	std::vector<Pos> _path;
	std::vector<Pos> _requestPath;
	uint16 _pathIndex = 0;
	bool bRequestPath = false;
	uint16 _pathFailCount = 0;
	void HandleAsyncFindPath();

	uint32 mapXSize = 0;
	uint32 mapYSize = 0;
};

