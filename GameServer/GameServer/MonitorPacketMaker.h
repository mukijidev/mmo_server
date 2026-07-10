#pragma once
#include "Type.h"
class CPacket;

void MP_SS_MONITOR_LOGIN(CPacket* packet, int serverNo);
void MP_SS_MONITOR_DATA_UPDATE(CPacket* pacekt, uint8 dataType, int dataValue, int timeStamp);
void MP_SS_MONITOR_SECTOR(CPacket* packet, uint8 gridId, uint8 w, uint8 h, const uint8* cells);