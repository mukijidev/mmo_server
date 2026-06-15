#include "PacketHeader.h"
#include "Packet.h"
#include "SerializeBuffer.h"
#include "Type.h"
#include "MonitorPacketMaker.h"
#include "MonitorPacket.h"


void MP_CS_MONITOR_VALUE(CPacket* packet, uint16 sessionNumPerSector[50][50],
	double cpuUsageTotal, double cpuUsageUser, double cpuUsageKernel,
	double processUserAllocMemory, double processNonPageMemory,
	double availableMemory, double nonPagedMemory,
	int64 totalAccept, int64 acceptTps, int64 sendTps, int64 recvTps, int64 sessionNum,
	int64 totalDisconnect, int64 loginTotal, int64 sectorMoveTotal, int64 messageTotal,
	int64 heartbeatTotal, int64 averageRtt, int64 lastRtt)
{
	LanHeader header;
	packet->PutData((char*)&header, sizeof(LanHeader));

	/*uint16 type = PACKET_CS_MONITOR;
	*packet << type;*/

	for (int i = 0; i < 50; i++)
	{
		for (int j = 0; j < 50; j++)
		{
			*packet << sessionNumPerSector[i][j];
		}
	}

	*packet << cpuUsageTotal;
	*packet << cpuUsageUser;
	*packet << cpuUsageKernel;
	*packet << processUserAllocMemory;
	*packet << processNonPageMemory;
	*packet << availableMemory;
	*packet << nonPagedMemory;
	*packet << totalAccept;
	*packet << acceptTps;
	*packet << sendTps;
	*packet << recvTps;
	*packet << sessionNum;
	*packet << totalDisconnect;
	*packet << loginTotal;
	*packet << sectorMoveTotal;
	*packet << messageTotal;
	*packet << heartbeatTotal;
	*packet << averageRtt;
	*packet << lastRtt;

	uint16 len = (uint16)(packet->GetDataSize() - sizeof(LanHeader));
	memcpy(packet->GetBufferPtr() + LAN_HEADER_SIZE_INDEX, (void*)&len, sizeof(uint16));
}
