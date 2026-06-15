#pragma once
#include "Type.h"
#define NET_HEADER_SIZE_INDEX 1

#pragma pack (push, 1)
struct NetHeader {
	uint8 _code;
	uint16 _len;
	uint8 _randKey;
	uint8 _checkSum;
};
#pragma pack (pop)

#define LAN_HEADER_SIZE_INDEX 0 
struct LanHeader {
	uint16 _len;
};
