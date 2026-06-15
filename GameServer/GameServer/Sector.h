#pragma once
#include "Type.h"
#include <unordered_set>
#include <vector>
#include <map>
#define AROUND_SEC_NUM 9
#define STRAIGHT_SEC_CHANGE_NUM 3
#define DIAGONAL_SEC_CHANGE_NUM 5
#define MOVE_DIR_LL			0
#define MOVE_DIR_LU			1
#define MOVE_DIR_UU			2
#define MOVE_DIR_RU			3
#define MOVE_DIR_RR			4
#define MOVE_DIR_RD			5
#define MOVE_DIR_DD			6
#define MOVE_DIR_LD			7
#define MOVE_DIR_MAX           8


extern std::map<std::pair<int8, int8>, int8> diffToDirection;

class FieldObject;

class Sector
{
public:
	uint8 X;
	uint8 Y;
	// 주변 8개 + 본인
	Sector* _around[AROUND_SEC_NUM];
	uint8 aroundSectorNum;
	// 왼쪽이동 3개
	Sector* left[STRAIGHT_SEC_CHANGE_NUM];
	uint8 leftSectorNum;
	// 오른쪽이동 3개
	Sector* right[STRAIGHT_SEC_CHANGE_NUM];
	uint8 rightSectorNum;
	// 위쪽 이동3개
	Sector* up[STRAIGHT_SEC_CHANGE_NUM];
	uint8 upSectorNum;
	// 아래 이동 3개
	Sector* down[STRAIGHT_SEC_CHANGE_NUM];
	uint8 downSectorNum;
	// 위 오른쪽 이동 5개
	Sector* upRight[DIAGONAL_SEC_CHANGE_NUM];
	uint8 upRightSectorNum;
	// 위 왼쪽 이동 5개
	Sector* upLeft[DIAGONAL_SEC_CHANGE_NUM];
	uint8 upLeftSectorNum;
	// 아래 오른쪽 이동 5개
	Sector* downRight[DIAGONAL_SEC_CHANGE_NUM];
	uint8 downRightSectorNum;
	// 아래 왼쪽 이동 5개
	Sector* downLeft[DIAGONAL_SEC_CHANGE_NUM];
	uint8 downLeftSectorNum;

	std::vector<FieldObject*> fieldObjectVector;
};

