#include "Sector.h"

std::map<std::pair<int8, int8>, int8> diffToDirection =
{
	{{0,-1}, MOVE_DIR_LL},
	{{-1,-1}, MOVE_DIR_LU},
	{{-1,0}, MOVE_DIR_UU},
	{{-1,1}, MOVE_DIR_RU},
	{{0,1}, MOVE_DIR_RR},
	{{1,1}, MOVE_DIR_RD},
	{{1,0}, MOVE_DIR_DD},
	{{1,-1}, MOVE_DIR_LD},
};