#include "PacketHeader.h"
#include "Packet.h"
#include "SerializeBuffer.h"
#include "Type.h"
#include "GameServer.h"
using namespace std;


void GameServer::MP_SC_MONITOR_TOOL_DATA_UPDATE(CPacket* packet, uint8& dataType, int& dataValue, int& timeStamp)
{
	//NetHeader header;
	//header._code = clientPacketCode;
	//header._randKey = rand();
	//packet->PutData((char*)&header, sizeof(NetHeader));
	//uint16 type = PACKET_SS_MONITOR_DATA_UPDATE;
	//*packet << type << dataType << dataValue << timeStamp;
	////*packet << dataType;
	////*packet << dataValue;
	//uint16 len = (uint16)(packet->GetDataSize() - sizeof(NetHeader));
	//memcpy(packet->GetBufferPtr() + NET_HEADER_SIZE_INDEX, (void*)&len, sizeof(uint16));
}

uint8** GameServer::LoadMapData(string filePath, uint32 mapYSize, uint32 mapXSize)
{
	//TODO: lobbymap.data 가져와서	
	uint8** map = new uint8 * [mapYSize];
	for (uint32 i = 0; i < mapYSize; i++)
	{
		map[i] = new uint8[mapXSize];
	}

	FILE* f;
	fopen_s(&f, filePath.c_str(), "rb");
	if (f == nullptr)
	{
		printf("Cannot open file : %s\n", filePath.c_str());
		LOG(L"GameServer", LogLevel::Error, L"Cannot open file : %s", filePath.c_str());
		return nullptr;
	}

	for (uint32 i = 0; i < mapYSize; i++)
	{
		fread(map[i], sizeof(uint8), mapXSize, f);
	}

	fclose(f);
	return map;
}

void GameServer::MP_SS_MONITOR_LOGIN(CPacket* packet, int& serverNo)
{
	//NetHeader header;
	//header._code = clientPacketCode;
	//header._randKey = rand();
	//packet->PutData((char*)&header, sizeof(NetHeader));
	//uint16 type = PACKET_SS_MONITOR_LOGIN;
	//*packet << type << serverNo;
	//uint16 len = (uint16)(packet->GetDataSize() - sizeof(NetHeader));
	//memcpy(packet->GetBufferPtr() + NET_HEADER_SIZE_INDEX, (void*)&len, sizeof(uint16));
}

