#include "PacketHeader.h"
#include "Packet.h"
#include "SerializeBuffer.h"
#include "Type.h"
#include "MonitorPacketMaker.h"
#include "MonitorPacket.h"
#include "MonitorProtocol.h"

void MP_SS_MONITOR_LOGIN(CPacket* packet, int serverNo)
{
	LanHeader header;
	packet->PutData((char*)&header, sizeof(LanHeader));

	uint16 type = PACKET_SS_MONITOR_LOGIN;
	*packet << type << serverNo;

	uint16 len = (uint16)(packet->GetDataSize() - sizeof(LanHeader));
	memcpy(packet->GetBufferPtr() + LAN_HEADER_SIZE_INDEX, (void*)&len, sizeof(uint16));
}

void MP_SS_MONITOR_DATA_UPDATE(CPacket* packet, uint8 dataType, int dataValue, int timeStamp)
{
	LanHeader header;
	packet->PutData((char*)&header, sizeof(LanHeader));

	uint16 type = PACKET_SS_MONITOR_DATA_UPDATE;
	*packet << type << dataType << dataValue << timeStamp;

	uint16 len = (uint16)(packet->GetDataSize() - sizeof(LanHeader));
	memcpy(packet->GetBufferPtr() + LAN_HEADER_SIZE_INDEX, (void*)&len, sizeof(uint16));
}

void MP_SS_MONITOR_SECTOR(CPacket* packet, uint8 gridId, uint8 w, uint8 h, const uint8* cells)
{
	LanHeader header;
	packet->PutData((char*)&header, sizeof(LanHeader));

	uint16 type = PACKET_SS_MONITOR_SECTOR;
	*packet << type << gridId << w << h;
	packet->PutData((char*)cells, (int)w * (int)h);

	uint16 len = (uint16)(packet->GetDataSize() - sizeof(LanHeader));
	memcpy(packet->GetBufferPtr() + LAN_HEADER_SIZE_INDEX, (void*)&len, sizeof(uint16));
}

