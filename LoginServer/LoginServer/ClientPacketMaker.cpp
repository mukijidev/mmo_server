#include "PacketHeader.h"
#include "Packet.h"
#include "SerializeBuffer.h"
#include "Type.h"
#include "Data.h"
#include "LoginServer.h"

void LoginServer::MP_SC_MONITOR_TOOL_DATA_UPDATE(CPacket* packet, uint8& dataType, int& dataValue, int& timeStamp)
{
	//	NetHeader header;
	//	header._code = clientPacketCode;
	//	header._randKey = rand();
	//	packet->PutData((char*)&header, sizeof(NetHeader));
	//	uint16 type = PACKET_SS_MONITOR_DATA_UPDATE;
	//	*packet << type << dataType << dataValue << timeStamp;
	//	//*packet << dataType;
	//	//*packet << dataValue;
	//	uint16 len = (uint16)(packet->GetDataSize() - sizeof(NetHeader));
	//	memcpy(packet->GetBufferPtr() + NET_HEADER_SIZE_INDEX, (void*)&len, sizeof(uint16));
	//}
}
void LoginServer::MP_SS_MONITOR_LOGIN(CPacket* packet, int& serverNo)
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
