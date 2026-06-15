#include "PacketMaker.h"
#include "Data.h"

void MP_SC_FIELD_MOVE(CPacket* packet, uint8& status, uint16& fieldID)
{
	NetHeader header;
	header._code = Data::serverPacketCode;
	header._randKey = rand();
	packet->PutData((char*)&header, sizeof(NetHeader));

	uint16 type = PACKET_SC_GAME_RES_FIELD_MOVE;
	*packet << type << status << fieldID;

	uint16 len = (uint16)(packet->GetDataSize() - sizeof(NetHeader));
	memcpy(packet->GetBufferPtr() + NET_HEADER_SIZE_INDEX, (void*)&len, sizeof(uint16));
}

void MP_SC_SPAWN_MY_CHARACTER(CPacket* packet, PlayerInfo& playerInfo, FVector& spawnLocation, FRotator& SpawnRotation)
{
	NetHeader header;
	header._code = Data::serverPacketCode;
	header._randKey = rand();
	packet->PutData((char*)&header, sizeof(NetHeader));

	uint16 type = PACKET_SC_GAME_SPAWN_MY_CHARACTER;
	*packet << type << playerInfo << spawnLocation << SpawnRotation;

	uint16 len = (uint16)(packet->GetDataSize() - sizeof(NetHeader));
	memcpy(packet->GetBufferPtr() + NET_HEADER_SIZE_INDEX, (void*)&len, sizeof(uint16));
}

void MP_SC_SPAWN_OTHER_CHARACTER(CPacket* packet, PlayerInfo& playerInfo, FVector& spawnLocation, FRotator& SpawnRotation)
{
	NetHeader header;
	header._code = Data::serverPacketCode;
	header._randKey = rand();
	packet->PutData((char*)&header, sizeof(NetHeader));

	uint16 type = PACKET_SC_GAME_SPAWN_OTHER_CHAACTER;
	*packet << type << playerInfo << spawnLocation << SpawnRotation;


	uint16 len = (uint16)(packet->GetDataSize() - sizeof(NetHeader));
	memcpy(packet->GetBufferPtr() + NET_HEADER_SIZE_INDEX, (void*)&len, sizeof(uint16));
}

//void MP_SC_GAME_RES_CHARACTER_MOVE(CPacket* packet, int64& charaterNo, FVector& Destination, FRotator& StartRotation)
//{
//	NetHeader header;
//	header._code = Data::serverPacketCode;
//	header._randKey = rand();
//	packet->PutData((char*)&header, sizeof(NetHeader));
//
//	uint16 type = PACKET_SC_GAME_RES_CHARACTER_MOVE;
//	*packet << type << charaterNo << Destination << StartRotation;
//
//	uint16 len = (uint16)(packet->GetDataSize() - sizeof(NetHeader));
//	memcpy(packet->GetBufferPtr() + NET_HEADER_SIZE_INDEX, (void*)&len, sizeof(uint16));
//}
//


void MP_SC_GAME_RES_DAMAGE(CPacket* packet, int32& AttackerType, int64& AttackerID, int32& targetType, int64& TargetID, int32& Damage)
{
	NetHeader header;
	header._code = Data::serverPacketCode;
	header._randKey = rand();
	packet->PutData((char*)&header, sizeof(NetHeader));

	uint16 type = PACKET_SC_GAME_RES_DAMAGE;
	*packet << type << AttackerType << AttackerID << targetType << TargetID << Damage;

	uint16 len = (uint16)(packet->GetDataSize() - sizeof(NetHeader));
	memcpy(packet->GetBufferPtr() + NET_HEADER_SIZE_INDEX, (void*)&len, sizeof(uint16));
}

void MP_SC_GAME_RES_CHARACTER_SKILL(CPacket* packet, int64& CharacterID, FVector& StartLocation, FRotator& StartRotation, int32& SkillID)
{
	NetHeader header;
	header._code = Data::serverPacketCode;
	header._randKey = rand();
	packet->PutData((char*)&header, sizeof(NetHeader));

	uint16 type = PACKET_SC_GAME_RES_CHARACTER_SKILL;
	*packet << type << CharacterID << StartLocation << StartRotation << SkillID;

	uint16 len = (uint16)(packet->GetDataSize() - sizeof(NetHeader));
	memcpy(packet->GetBufferPtr() + NET_HEADER_SIZE_INDEX, (void*)&len, sizeof(uint16));
}


void MP_SC_GAME_RES_MONSTER_SKILL(CPacket* packet, int64& MonsterNO, FVector& StartPostion, FRotator& StartRotation, int32& SkillID)
{
	NetHeader header;
	header._code = Data::serverPacketCode;
	header._randKey = rand();
	packet->PutData((char*)&header, sizeof(NetHeader));

	uint16 type = PACKET_SC_GAME_RES_MONSTER_SKILL;
	*packet << type << MonsterNO << StartPostion << StartRotation << SkillID;

	uint16 len = (uint16)(packet->GetDataSize() - sizeof(NetHeader));
	memcpy(packet->GetBufferPtr() + NET_HEADER_SIZE_INDEX, (void*)&len, sizeof(uint16));
}

void MP_SC_SPAWN_MONSTER(CPacket* packet, MonsterInfo& monsterInfo, FVector& spawnLocation, FRotator& SpawnRotation)
{
	NetHeader header;
	header._code = Data::serverPacketCode;
	header._randKey = rand();
	packet->PutData((char*)&header, sizeof(NetHeader));

	uint16 type = PACKET_SC_GAME_SPAWN_MONSTER;
	*packet << type << monsterInfo << spawnLocation << SpawnRotation;

	uint16 len = (uint16)(packet->GetDataSize() - sizeof(NetHeader));
	memcpy(packet->GetBufferPtr() + NET_HEADER_SIZE_INDEX, (void*)&len, sizeof(uint16));
}

void MP_SC_MONSTER_MOVE(CPacket* packet, int64& monsterId, FVector& currentPos, std::vector<Pos>& path, uint16& startIndex)
{
	NetHeader header;
	header._code = Data::serverPacketCode;
	header._randKey = rand();
	packet->PutData((char*)&header, sizeof(NetHeader));
	
	uint16 type = PACKET_SC_GAME_MONSTER_MOVE;
	*packet << type << monsterId << currentPos << (uint16)path.size() << startIndex;

	for (auto& pos : path)
	{
		*packet << pos;
	}
	
	uint16 len = (uint16)(packet->GetDataSize() - sizeof(NetHeader));
	memcpy(packet->GetBufferPtr() + NET_HEADER_SIZE_INDEX, (void*)&len, sizeof(uint16));
}

void MP_SC_GAME_RSE_CHARACTER_STOP(CPacket* packet, int64& characterID, FVector& position, FRotator& rotation)
{
	NetHeader header;
	header._code = Data::serverPacketCode;
	header._randKey = rand();
	packet->PutData((char*)&header, sizeof(NetHeader));

	uint16 type = PACKET_SC_GAME_RES_CHARACTER_STOP;
	*packet << type << characterID << position << rotation;

	uint16 len = (uint16)(packet->GetDataSize() - sizeof(NetHeader));
	memcpy(packet->GetBufferPtr() + NET_HEADER_SIZE_INDEX, (void*)&len, sizeof(uint16));
}

void MP_SC_GAME_RES_MONSTER_STOP(CPacket* packet, int64& monsterID, FVector& position, FRotator& rotation)
{
	NetHeader header;
	header._code = Data::serverPacketCode;
	header._randKey = rand();
	packet->PutData((char*)&header, sizeof(NetHeader));

	uint16 type = PACKET_SC_GAME_RES_MONSTER_STOP;
	*packet << type << monsterID << position << rotation;

	uint16 len = (uint16)(packet->GetDataSize() - sizeof(NetHeader));
	memcpy(packet->GetBufferPtr() + NET_HEADER_SIZE_INDEX, (void*)&len, sizeof(uint16));
}

void MP_SC_GAME_RES_CHARACTER_DEATH(CPacket* packet, int64& characterID, FVector& DeathLocation, FRotator& DeathRotation)
{
	NetHeader header;
	header._code = Data::serverPacketCode;
	header._randKey = rand();
	packet->PutData((char*)&header, sizeof(NetHeader));

	uint16 type = PACKET_SC_GAME_RES_CHARACTER_DEATH;
	*packet << type << characterID << DeathLocation << DeathRotation;

	uint16 len = (uint16)(packet->GetDataSize() - sizeof(NetHeader));
	memcpy(packet->GetBufferPtr() + NET_HEADER_SIZE_INDEX, (void*)&len, sizeof(uint16));
}

void MP_SC_GAME_RES_MONSTER_DEATH(CPacket* packet, int64& monsterID, FVector& DeathLocation, FRotator& DeathRotation)
{
	NetHeader header;
	header._code = Data::serverPacketCode;
	header._randKey = rand();
	packet->PutData((char*)&header, sizeof(NetHeader));

	uint16 type = PACKET_SC_GAME_RES_MONSTER_DEATH;
	*packet << type << monsterID << DeathLocation << DeathRotation;

	uint16 len = (uint16)(packet->GetDataSize() - sizeof(NetHeader));
	memcpy(packet->GetBufferPtr() + NET_HEADER_SIZE_INDEX, (void*)&len, sizeof(uint16));
}

void MP_SC_GAME_DESPAWN_OTHER_CHARACTER(CPacket* packet, int64& characterNO)
{
	NetHeader header;
	header._code = Data::serverPacketCode;
	header._randKey = rand();
	packet->PutData((char*)&header, sizeof(NetHeader));

	uint16 type = PACKET_SC_GAME_DESPAWN_OTHER_CHARACTER;
	*packet << type << characterNO;

	uint16 len = (uint16)(packet->GetDataSize() - sizeof(NetHeader));
	memcpy(packet->GetBufferPtr() + NET_HEADER_SIZE_INDEX, (void*)&len, sizeof(uint16));
}

void MP_SC_LOGIN(CPacket* packet, int64& AccountNo, uint8& Status)
{
	NetHeader header;
	header._code = Data::serverPacketCode;
	header._randKey = rand();
	packet->PutData((char*)&header, sizeof(NetHeader));

	uint16 type = PACKET_SC_GAME_RES_LOGIN;
	*packet << type << AccountNo << Status;

	uint16 len = (uint16)(packet->GetDataSize() - sizeof(NetHeader));
	memcpy(packet->GetBufferPtr() + NET_HEADER_SIZE_INDEX, (void*)&len, sizeof(uint16));
}

void MP_SC_GAME_RES_SIGN_UP(CPacket* packet, uint8& Status)
{
	NetHeader header;
	header._code = Data::serverPacketCode;
	header._randKey = rand();
	packet->PutData((char*)&header, sizeof(NetHeader));

	uint16 type = PACKET_SC_GAME_RES_SIGN_UP;
	*packet << type << Status;

	uint16 len = (uint16)(packet->GetDataSize() - sizeof(NetHeader));
	memcpy(packet->GetBufferPtr() + NET_HEADER_SIZE_INDEX, (void*)&len, sizeof(uint16));
}

void MP_SC_PLAYER_LIST(CPacket* packet, std::vector<PlayerInfo>& playerInfos)
{
	NetHeader header;
	header._code = Data::serverPacketCode;
	header._randKey = rand();
	packet->PutData((char*)&header, sizeof(NetHeader));

	uint16 type = PACKET_SC_GAME_RES_PLAYER_LIST;
	*packet << type << (uint8)playerInfos.size();

	for (auto& playerInfo : playerInfos)
	{
		*packet << playerInfo;
	}

	uint16 len = (uint16)(packet->GetDataSize() - sizeof(NetHeader));
	memcpy(packet->GetBufferPtr() + NET_HEADER_SIZE_INDEX, (void*)&len, sizeof(uint16));
}

void MP_SC_SELECT_PLAYER(CPacket* packet, uint8& Status)
{
	NetHeader header;
	header._code = Data::serverPacketCode;
	header._randKey = rand();
	packet->PutData((char*)&header, sizeof(NetHeader));

	uint16 type = PACKET_SC_GAME_RES_SELECT_PLAYER;
	*packet << type << Status;

	uint16 len = (uint16)(packet->GetDataSize() - sizeof(NetHeader));
	memcpy(packet->GetBufferPtr() + NET_HEADER_SIZE_INDEX, (void*)&len, sizeof(uint16));
}

void MP_SC_CREATE_PLAYER(CPacket* packet, uint8& Status, PlayerInfo& playerInfo)
{
	NetHeader header;
	header._code = Data::serverPacketCode;
	header._randKey = rand();
	packet->PutData((char*)&header, sizeof(NetHeader));

	uint16 type = PACKET_SC_GAME_RES_CREATE_PLAYER;
	*packet << type << Status << playerInfo;

	uint16 len = (uint16)(packet->GetDataSize() - sizeof(NetHeader));
	memcpy(packet->GetBufferPtr() + NET_HEADER_SIZE_INDEX, (void*)&len, sizeof(uint16));
}

void MP_SC_GAME_DESPAWN_MONSTER(CPacket* packet, int64& monsterId)
{
	NetHeader header;
	header._code = Data::serverPacketCode;
	header._randKey = rand();
	packet->PutData((char*)&header, sizeof(NetHeader));

	uint16 type = PACKET_SC_GAME_DESPAWN_MONSTER;
	*packet << type << monsterId;

	uint16 len = (uint16)(packet->GetDataSize() - sizeof(NetHeader));
	memcpy(packet->GetBufferPtr() + NET_HEADER_SIZE_INDEX, (void*)&len, sizeof(uint16));
}

void MP_SC_FIND_PATH(CPacket* packet, int64 characterNo,  FVector& currentPos, std::vector<Pos>& path, uint16& startIndex)
{
	NetHeader header;
	header._code = Data::serverPacketCode;
	header._randKey = rand();
	packet->PutData((char*)&header, sizeof(NetHeader));
	
	uint16 type = PACKET_SC_GAME_RES_FIND_PATH;
	*packet << type << characterNo << currentPos << (uint16)path.size() << startIndex;
	//*packet << type << currentPos << startIndex << (uint16)path.size();

	for (auto& pos : path)
	{
		*packet << pos;
	}

	uint16 len = (uint16)(packet->GetDataSize() - sizeof(NetHeader));
	memcpy(packet->GetBufferPtr() + NET_HEADER_SIZE_INDEX, (void*)&len, sizeof(uint16));
}

void MP_SC_GAME_RES_EXP_CHANGE(CPacket* packet, uint16 Level, uint32 Exp)
{
	NetHeader header;
	header._code = Data::serverPacketCode;
	header._randKey = rand();
	packet->PutData((char*)&header, sizeof(NetHeader));
	
	uint16 type = PACKET_SC_GAME_RES_EXP_CHANGE;
	*packet << type << Level << Exp;
	
	uint16 len = (uint16)(packet->GetDataSize() - sizeof(NetHeader));
	memcpy(packet->GetBufferPtr() + NET_HEADER_SIZE_INDEX, (void*)&len, sizeof(uint16));
}

void MP_SC_GAME_LEVEL_UP_OTHER_CHARACTER(CPacket* packet, int64& characterNO, uint16& Level)
{
	NetHeader header;
	header._code = Data::serverPacketCode;
	header._randKey = rand();
	packet->PutData((char*)&header, sizeof(NetHeader));
	
	uint16 type = PACKET_SC_GAME_RES_LEVEL_UP_OTHER_CHARACTER;
	*packet << type << characterNO << Level;
	
	uint16 len = (uint16)(packet->GetDataSize() - sizeof(NetHeader));
	memcpy(packet->GetBufferPtr() + NET_HEADER_SIZE_INDEX, (void*)&len, sizeof(uint16));
}

