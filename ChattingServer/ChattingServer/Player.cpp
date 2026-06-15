#include "Player.h"
using namespace std;


void Player::Init(int64 sessionId)
{
	_sessionId = sessionId;
	_lastRecvTime = 0;
	_bLogined = false;
	memset(NickName, 0, sizeof(NickName));
}




