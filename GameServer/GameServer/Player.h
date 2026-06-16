#pragma once
#include "Session.h"
#include <unordered_map>
#include <algorithm>
#include "Type.h"
#include <map>
#include "FieldObject.h"
#include "JumpPointSearch.h"
#include <vector>

const double PLAYER_Z_VALUE = 95.2f;

class Player : public FieldObject
{
	friend class Monster;
	friend class GameServer;
	friend class BasePacketHandleThread;
	friend class GuardianFieldThread;
	friend class LobbyFieldThread;
	friend class SpiderFieldThread;
	friend class LoginGameThread;
	friend class FieldPacketHandleThread;

public:
	Player(FieldPacketHandleThread* field, uint16 objectType, int64 sessionId);

private:
	int64 _sessionId;
	int64 accountNo = 0;
	uint32 _lastRecvTime = 0;
	bool _bLogined = false;
	int32 _damage = 10;

	//TODO: 파일로빼기
private:
	const float _attackRange = 250.f;
	const float _attackRangeSquare = _attackRange * _attackRange;
	const uint32 _attackCoolDown = 500; //클라 0.6s
	uint32 _lastAttackTime = 0;

public:
	FVector Position;
	FRotator Rotation;
	PlayerInfo playerInfo;

	std::vector<PlayerInfo> playerInfos;
	void Update(float deltaTime);
	void SetDestination(const FVector& NewDestination);
	void StopMove();
	void OnLeave();
	void OnFieldChange();

	int64 GetSessionId() { return _sessionId; };
public:
	//void HandleCharacterMove(FVector destination);
	void HandleCharacterSkill(FVector startLocation, FRotator startRotation, int32 skillId);
	void HandleCharacterStop(FVector position, FRotator rotation);
	void HandleCharacterAttack(int32 attackerType, int64 attackerId, int32 targetType, int64 targetId);
	void HandleAsyncFindPath();
private:
	FVector _destination;
	float _speed;
	bool bMoving = false;
	void Move(float deltaTime);


	/// <summary>
	/// 
	/// </summary>
	/// <param name="damage"></param>
	/// <returns> return true when death </returns>
	bool TakeDamage(int32 damage);


public:
	void ProcessSectorChange(Sector* newSector);
	void AddSector(Sector* newSEctor);
	void RemoveSector(Sector* newSector);
	std::vector<Pos> _path;
	std::vector<Pos> _requestPath;
	uint16 _pathIndex = 0;
	bool _bRequestPath = false;
	FVector _lastValidPosition;
	uint32 _lastRequestPathTime = 0;
};

