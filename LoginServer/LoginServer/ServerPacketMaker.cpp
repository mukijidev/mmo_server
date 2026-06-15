#include "PacketHeader.h"
#include "Packet.h"
#include "SerializeBuffer.h"
#include "Type.h"
#include "Data.h"
#include "LoginServer.h"

void LoginServer::MP_SC_LOGIN(CPacket* packet,  int64& accountNo, uint8& status,WCHAR* gameServerIP, uint16& gameServerPort, WCHAR* chatServerIP, uint16& chatServerPort)
{
	NetHeader header;
	header._code = serverPacketCode;
	header._randKey = rand();
	packet->PutData((char*)&header, sizeof(NetHeader));

	uint16 type = PACKET_SC_LOGIN_RES_LOGIN;
	*packet << type;
	*packet << accountNo;
	*packet << status;

	packet->PutData((char*)gameServerIP, IP_LEN * sizeof(WCHAR));
	*packet << gameServerPort;
	packet->PutData((char*)chatServerIP, IP_LEN * sizeof(WCHAR));
	*packet << chatServerPort;

	uint16 len = (uint16)(packet->GetDataSize() - sizeof(NetHeader));
	memcpy(packet->GetBufferPtr() + NET_HEADER_SIZE_INDEX, (void*)&len, sizeof(uint16));
}

void LoginServer::MP_SC_ECHO(CPacket* packet)
{
	NetHeader header;
	header._code = serverPacketCode;
	header._randKey = rand();
	packet->PutData((char*)&header, sizeof(NetHeader));

	uint16 type = PACKET_SC_LOGIN_RES_ECHO;
	*packet << type;
	uint16 len = (uint16)(packet->GetDataSize() - sizeof(NetHeader));
	memcpy(packet->GetBufferPtr() + NET_HEADER_SIZE_INDEX, (void*)&len, sizeof(uint16));
}

