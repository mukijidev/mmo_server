#include "Type.h"
#include "SerializeBuffer.h"

CPacket& operator<<(CPacket& packet, FRotator& rot)
{
	packet << rot.Pitch << rot.Yaw << rot.Roll;
	return packet;
}

CPacket& operator>>(CPacket& packet, FRotator& rot)
{
	packet >> rot.Pitch >> rot.Yaw >> rot.Roll;
	return packet;
}

CPacket& operator<<(CPacket& packet, PlayerInfo& info)
{
	packet.PutData((char*)info.NickName, sizeof(TCHAR) * NICKNAME_LEN);
	packet << info.PlayerID << info.Class << info.Level << info.Exp << info.Hp;
	return packet;
}

CPacket& operator>>(CPacket& packet, PlayerInfo& info)
{
	packet.GetData((char*)info.NickName, sizeof(TCHAR) * NICKNAME_LEN);
	packet >> info.PlayerID >> info.Class >> info.Level >> info.Exp >> info.Hp;
	return packet;
}

CPacket& operator<<(CPacket& packet, MonsterInfo& info)
{
	packet << info.MonsterID << info.Type << info.Hp;
	return packet;
}

CPacket& operator>>(CPacket& packet, MonsterInfo& info)
{
	packet >> info.MonsterID >> info.Type >> info.Hp;
	return packet;
}


CPacket& operator>>(CPacket& packet, Pos& pos)
{
	packet >> pos.y >> pos.x;
	return packet;
}

CPacket& operator<<(CPacket& packet, Pos& pos)
{
	packet << pos.y << pos.x;
	return packet;
}

CPacket& operator<<(CPacket& packet, FVector& vec)
{
	packet << vec.X << vec.Y << vec.Z;
	return packet;
}

CPacket& operator>>(CPacket& packet, FVector& vec)
{
	packet >> vec.X >> vec.Y >> vec.Z;
	return packet;
}
