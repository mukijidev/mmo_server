#include "PacketHeader.h"
#include "Packet.h"
#include "SerializeBuffer.h"
#include "Type.h"
#include "Data.h"
#include "LoginServer.h"
#include "MonitorProtocol.h"

void LoginServer::MP_SS_MONITOR_TOOL_DATA_UPDATE(CPacket* packet, uint8& dataType, int& dataValue, int& timeStamp)
{
	LanHeader header;
	packet->PutData((char*)&header, sizeof(LanHeader));

	uint16 type = PACKET_SS_MONITOR_DATA_UPDATE;
	*packet << type << dataType << dataValue << timeStamp;

	uint16 len = (uint16)(packet->GetDataSize() - sizeof(LanHeader));
	memcpy(packet->GetBufferPtr() + LAN_HEADER_SIZE_INDEX, (void*)&len, sizeof(uint16));

	
}
void LoginServer::MP_SS_MONITOR_LOGIN(CPacket* packet, int& serverNo)
{
	LanHeader header;
	packet->PutData((char*)&header, sizeof(LanHeader));

	uint16 type = PACKET_SS_MONITOR_LOGIN;
	*packet << type << serverNo;

	uint16 len = (uint16)(packet->GetDataSize() - sizeof(LanHeader));
	memcpy(packet->GetBufferPtr() + LAN_HEADER_SIZE_INDEX, (void*)&len, sizeof(uint16));
}

void LoginServer::SendMonitorData(uint8 dataType, int value, int ts)
{
	CPacket* p = CPacket::Alloc();
	MP_SS_MONITOR_TOOL_DATA_UPDATE(p, dataType, value, ts);
	_monitorClient->SendPacket(p);
	CPacket::Free(p);
}
