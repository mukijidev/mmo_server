#pragma once
#include "Type.h"
#include <atomic>

class FieldPacketHandleThread;
class Sector;

/// <summary>
/// Player, Monster, Etc... 
/// Field에서 Update할 수있고 PacketBroadcast
/// </summary>
class FieldObject
{
	friend class FieldPacketHandleThread;

public:
	uint16 GetObjectType() { return _objectType; };
	int64 GetObjectId() { return _objectId; };

protected:
	FieldObject(FieldPacketHandleThread* field, uint16 objectType);
	virtual ~FieldObject();
	void SendPacket_Unicast(int64 objectId, CPacket* packet);
	void SendPacket_Around(CPacket* packet, bool bInclude = true);
	void SendPacket_Sector(Sector* sector, CPacket* packet);

	FieldObject* FindFieldObject(int64 objectId);
	void SetField(FieldPacketHandleThread* field) { _field = field; };

	virtual void Update(float deltaTime) = 0;
	FieldPacketHandleThread* GetField() { return _field; };

protected:
	int64 _objectId;
	uint16 _objectType;
	Sector* _currentSector;
	FieldPacketHandleThread* _field;
	uint16 _sectorYSize;
	uint16 _sectorXSize;
	bool bErase = false;
};

