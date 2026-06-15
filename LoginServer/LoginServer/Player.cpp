#include "Player.h"
using namespace std;



void Player::Init(int64 sessionId, WCHAR* ip)
{
	_lastRecvTime = timeGetTime();
	_sessionId = sessionId;
	_bLogined = false;
	memset(id, 0, sizeof(id));
	memset(nickName, 0, sizeof(nickName));

	wmemcpy(_ip, ip, wcslen(ip));
}



