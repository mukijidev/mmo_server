#pragma once
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>

#define ID_LEN 20
#define NICKNAME_LEN 20
#define PASS_LEN 20
#define PI 3.141592653589


class CPacket;
using int64 = signed long long;
using int32 = int;
using int16 = short;
using int8 = char;
using uint32 = unsigned int;
using uint16 = unsigned short;
using uint64 = unsigned long long;
using uint8 = unsigned char;

using TCHAR = WCHAR;

struct FVector
{
	double X;
	double Y;
	double Z;
};

CPacket& operator<<(CPacket& packet, FVector& vec);
CPacket& operator>>(CPacket& packet, FVector& vec);

struct FRotator
{
	double Pitch;
	double Yaw;
	double Roll;
};

CPacket& operator<<(CPacket& packet, FRotator& rot);
CPacket& operator>>(CPacket& packet, FRotator& rot);

struct PlayerInfo
{
	int64 PlayerID;
	TCHAR NickName[NICKNAME_LEN];
	uint16 Class;
	uint16 Level;
	uint32 Exp;
	int32 Hp;
};

CPacket& operator<<(CPacket& packet, PlayerInfo& info);
CPacket& operator>>(CPacket& packet, PlayerInfo& info);

struct MonsterInfo
{
	int64 MonsterID;
	uint16 Type;
	int32 Hp;
};

CPacket& operator<<(CPacket& packet, MonsterInfo& info);
CPacket& operator>>(CPacket& packet, MonsterInfo& info);

struct Pos {
	int y;
	int x;

	Pos& operator=(const Pos& other)
	{
		x = other.x;
		y = other.y;
		return *this;
	}

	Pos operator+(const Pos& other) {
		Pos ret;
		ret.y = y + other.y;
		ret.x = x + other.x;
		return ret;
	}

	Pos operator-(const Pos& other)
	{
		Pos ret;
		ret.y = y + other.y;
		ret.x = x + other.x;
		return ret;
	}

	bool operator<(const Pos& other) const {
		if (y < other.y) return true;
		if (y > other.y) return false;
		// y가 같을 경우 x를 비교
		return x < other.x;
	}

	bool operator==(const Pos& other) const {
		return y == other.y && x == other.x;
	}

	bool operator!=(const Pos& other) const {
		return y != other.y || x != other.x;
	}
};

CPacket& operator<<(CPacket& packet, Pos& pos);
CPacket& operator>>(CPacket& packet, Pos& pos);