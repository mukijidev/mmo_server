#include "FieldObject.h"
#include "Sector.h"
#include "GameServer.h"
#include "GameData.h"

using namespace std;

FieldObject::FieldObject(FieldPacketHandleThread* field, uint16 objectType)
	: _field(field), _objectType(objectType)
{
	static atomic<int64_t> objectIdGenerator = 0;
	_objectId = objectIdGenerator++;
}

FieldObject::~FieldObject()
{

}

void FieldObject::SendPacket_Unicast(int64 objectId, CPacket* packet)
{
	_field->SendPacket(objectId, packet);
}

void FieldObject::SendPacket_Around(CPacket* packet, bool bInclude)
{
	int aroundSectorNum = _currentSector->aroundSectorNum;
	Sector** around = _currentSector->_around;

	
	for (int i = 0; i < aroundSectorNum; i++)
	{
		vector<FieldObject*>& fieldObjecVector = around[i]->fieldObjectVector;

		for (FieldObject* o : fieldObjecVector)
		{
			if(o->GetObjectType() == TYPE_PLAYER)
			{
				Player* p = static_cast<Player*>(o);
				if (!bInclude && p->GetObjectId() == _objectId)
				{
					continue;
				}
				SendPacket_Unicast(p->GetSessionId(), packet);
			}
		}
	}
}

void FieldObject::SendPacket_Sector(Sector* sector, CPacket* packet)
{
	vector<FieldObject*>& fieldObjecVector = sector->fieldObjectVector;

	for (FieldObject* o : fieldObjecVector)
	{
		if (o->GetObjectType() == TYPE_PLAYER)
		{
			Player* p = static_cast<Player*>(o);
			SendPacket_Unicast(p->GetSessionId(), packet);
		}
	}
}

FieldObject* FieldObject::FindFieldObject(int64 objectId)
{
	return _field->FindFieldObject(objectId);
}
