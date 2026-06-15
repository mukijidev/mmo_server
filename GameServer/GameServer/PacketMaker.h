#pragma once

#include "Packet.h"
#include "Type.h"
#include "SerializeBuffer.h"
#include <vector>

struct Pos;

void MP_SC_FIELD_MOVE(CPacket* packet, uint8& status, uint16& fieldID);
void MP_SC_SPAWN_MY_CHARACTER(CPacket* packet, PlayerInfo& playerInfo, FVector& spawnLocation, FRotator& SpawnRotation);
void MP_SC_SPAWN_OTHER_CHARACTER(CPacket* packet, PlayerInfo& playerInfo, FVector& spawnLocation, FRotator& SpawnRotation);
//void MP_SC_GAME_RES_CHARACTER_MOVE(CPacket* packet, int64& charaterNo, FVector& Destination, FRotator& StartRotation);
void MP_SC_GAME_RES_DAMAGE(CPacket* packet, int32& AttackerType, int64& AttackerID, int32& targetType,
	int64& TargetID, int32& Damage);
void MP_SC_GAME_RES_CHARACTER_SKILL(CPacket* packet, int64& CharacterID,
	FVector& StartLocation, FRotator& StartRotation, int32& SkillID);
void MP_SC_GAME_RES_MONSTER_SKILL(CPacket* packet, int64& MonsterNO, FVector& StartPostion, FRotator& StartRotation, int32& SkillID);
void MP_SC_SPAWN_MONSTER(CPacket* packet, MonsterInfo& monsterInfo, FVector& spawnLocation, FRotator& SpawnRotation);
void MP_SC_MONSTER_MOVE(CPacket* packet, int64& monsterId, FVector& currentPos, std::vector<Pos>& path, uint16& startIndex);
void MP_SC_GAME_RSE_CHARACTER_STOP(CPacket* packet, int64& characterID, FVector& position, FRotator& rotation);
void MP_SC_GAME_RES_MONSTER_STOP(CPacket* packet, int64& monsterID, FVector& position, FRotator& rotation);
void MP_SC_GAME_RES_CHARACTER_DEATH(CPacket* packet, int64& characterID, FVector& DeathLocation, FRotator& DeathRotation);
void MP_SC_GAME_RES_MONSTER_DEATH(CPacket* packet, int64& monsterID, FVector& DeathLocation, FRotator& DeathRotation);
void MP_SC_GAME_DESPAWN_OTHER_CHARACTER(CPacket* packet, int64& characterNO);
void MP_SC_LOGIN(CPacket* packet, int64& AccountNo, uint8& Status);
void MP_SC_GAME_RES_SIGN_UP(CPacket* packet, uint8& Status);
void MP_SC_PLAYER_LIST(CPacket* packet, std::vector<PlayerInfo>& playerInfos);
void MP_SC_SELECT_PLAYER(CPacket* packet, uint8& Status);
void MP_SC_CREATE_PLAYER(CPacket* packet, uint8& Status, PlayerInfo& playerInfo);
void MP_SC_GAME_DESPAWN_MONSTER(CPacket* packet, int64& monsterId);
void MP_SC_FIND_PATH(CPacket* packet, int64 characterNo, FVector& currentPos, std::vector<Pos>& path, uint16& startIndex);
void MP_SC_GAME_RES_EXP_CHANGE(CPacket* packet, uint16 Level, uint32 Exp);
void MP_SC_GAME_LEVEL_UP_OTHER_CHARACTER(CPacket* packet, int64& characterNO, uint16& Level);