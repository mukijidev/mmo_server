#include "JumpPointSearch.h"
using namespace std;
#define DIRECTION_NUM 8


Pos dir[DIRECTION_NUM] = {
	Pos{-1,0}, // 위 
	Pos{0, 1},  //오른쪽
	Pos{1, 0}, // 아래
	Pos{0, -1}, // 왼쪽
	Pos{-1,1},// 위오른쪽
	Pos {1,1}, // 오른쪽 아래
	Pos{1,-1}, //왼쪽아래
	Pos{-1,-1} // 왼쪽위
};

int gVal[DIRECTION_NUM] = {
	10, // 위
	10, // 오른쪽
	10, // 아래
	10, // 왼쪽
	14, // 대각 위오른쪽
	14, // 대각 오른쪽 아래
	14, //대각  왼쪽아래
	14// 대각 왼쪽위 
};

#define df_DIR_U 0
#define df_DIR_R 1
#define df_DIR_D 2
#define df_DIR_L 3
#define df_DIR_UR 4
#define df_DIR_RD 5
#define df_DIR_LD 6
#define df_DIR_LU 7


JumpPointSearch::JumpPointSearch(uint8** coarse, int32 cNy, int32 cNx, uint8** fine, int32 fNy, int32 fNx, int cellSize)
{
	_originMap = coarse;
	_mapYSize = cNy;
	_mapXSize = cNx;
	_fineMap = fine;
	_C = cellSize;
	_fineY = fNy;
	_fineX = fNx;

	_jpsMap = new uint8 * [_mapYSize];
	for (int i = 0; i < _mapYSize; i++)
	{
		_jpsMap[i] = new uint8[_mapXSize];
		memset(_jpsMap[i], 0, sizeof(uint8) * _mapXSize);
	}
}

JumpPointSearch::~JumpPointSearch()
{
	for(int i = 0; i < _mapYSize; i++)
	{
		delete[] _jpsMap[i];
	}
	delete[] _jpsMap;
}

void JumpPointSearch::FindPath(Pos start, Pos end, OUT std::vector<Pos>& returnPath)
{
	if (start == end)
	{
		return;
	}
	_start = start;
	_dest = end;
	if(_originMap[_dest.y][_dest.x] == OBSTACLE)
	{
		return;
	}

	//길찾기 범위 초과면 거부
	int dist = abs(_start.x - _dest.x) + abs(_start.y - _dest.y);
	if (dist > MAX_PATH_RANGE_CELLS)
		return;

	std::vector<Pos> path;
	//First : 맵 초기화하고
	for (int i = 0; i < _mapYSize; ++i)
	{
		memcpy(_jpsMap[i], _originMap[i], sizeof(uint8) * _mapXSize);
	}

	Node* startNode = CreateStartNode(_start);

	uint32 findRange = (abs(_start.x - _dest.x) + abs(_start.y - _dest.y)) * 1.5;
	//시작지점은  8방향 검사하고
	for (int i = 0; i < DIRECTION_NUM; i++)
	{
		CheckAndMakeCorner(startNode, _start, i, findRange);
	}
	
	if (openList.size() == 0)
	{
		//길 못찾음
		return;
	}
	int expanded = 0;
	while (true)
	{
		if (++expanded > MAX_NODES)
		{
			ClearNode();
			return;
		}

		//정렬 하고
		openList.sort(NodeComparator());
		if(openList.size() == 0)
		{
			ClearNode();
			return;
		}

		Node* nodeNow = openList.front();
		Pos posNow = nodeNow->_pos;

		if (posNow == _dest)
		{
			//현재 Node가 목적지이면
			while (true)
			{
				path.push_back(nodeNow->_pos);
				if (nodeNow->_pos == _start)
				{
					break;
				}
				nodeNow = nodeNow->_parent;
			}
			std::reverse(path.begin(), path.end());
			FindShortestPath(path, returnPath);
			ClearNode();
			return;
		}
		// 아니면

		for (int i = 0; i < 8; i++)
		{
			if (nodeNow->directionArr[i] == 0)
			{
				continue;
			}
			// 이번 노드의 탐색방향으로 탐색
			uint32 findRangeNodeNow = (abs(posNow.x - _dest.x) + abs(posNow.y - _dest.y)) * 1.5;
			if (CheckAndMakeCorner(nodeNow, posNow, i, findRangeNodeNow))
				break;
		}
		openList.pop_front();
		SetMap(posNow, CLOSE);
		closeList.push_back(nodeNow);
	}
}


Node* JumpPointSearch::CreateStartNode(Pos pos)
{
	int h = GetH(_start, _dest);
	Node* startNode = new Node;
	startNode->_pos = _start;
	startNode->_f = h;
	startNode->_h = h;

	//openList.push_back(startNode); push back 안하고
	//_debugNodeList[_start] = startNode;
	SetMap(_start, OPEN);
	//_first = false;
	return startNode;
}

void JumpPointSearch::SetMap(Pos& pos, uint8 value)
{
	//if (pos.y < 0 || pos.x < 0 || pos.y >= _mapYSize || pos.x >= _mapXSize)
	//{
	//	return;
	//}

	//if (_started)
	//{
	//	if (pos == _start || pos == _dest)
	//	{
	//		return;
	//	}
	//}

	//if (value == BRESENHAM)
	//{
	//	tile[pos.y][pos.x] = value;
	//	return;
	//}

	//if (value != ROUTE)
	//{
	//	//경로가 아닌것을
	//	// 오픈인곳을 close가아닌것으로 변경하려하면
	//	if (GetMapValue(pos) == OPEN && value != CLOSE)
	//	{
	//		return;
	//	}

	//	// close인것을 변경하려하면
	//	if (GetMapValue(pos) == CLOSE)
	//	{
	//		return;
	//	}
	//}

	_jpsMap[pos.y][pos.x] = value;
}

int JumpPointSearch::CheckUp(Pos pos)
{
	// 위로 한칸올라온거임
	int ret = 0;

	if (pos.y == 0)
	{
		return ret;
	}

	if (pos.x >= 1)
	{
		// 왼쪽 OBSTACLE, 왼쪽위가 NONE
		if (GetMapValue(pos + dir[df_DIR_L]) == OBSTACLE && GetMapValue(pos + dir[df_DIR_LU]) != OBSTACLE)
		{
			ret |= 1 << df_DIR_LU;
			ret |= 1 << df_DIR_U;
		}
	}

	if (pos.x <= _mapXSize - 2)
	{
		// 오룬쪽이 OBSTACLE이고 오른쪽위가 NONE
		if (GetMapValue(pos + dir[df_DIR_R]) == OBSTACLE && GetMapValue(pos + dir[df_DIR_UR]) != OBSTACLE)
		{
			ret |= 1 << df_DIR_UR;
			ret |= 1 << df_DIR_U;
		}
	}

	return ret;
}

int JumpPointSearch::CheckRight(Pos pos)
{
	int ret = 0;

	if (pos.x == _mapXSize - 1)
	{
		return ret;
	}

	if (pos.y >= 1)
	{
		// 위가 OBSTACLE이고 위 오른쪽이 NONE
		if (GetMapValue(pos + dir[df_DIR_U]) == OBSTACLE && GetMapValue(pos + dir[df_DIR_UR]) != OBSTACLE)
		{
			ret |= 1 << df_DIR_UR;
			ret |= 1 << df_DIR_R;
		}
	}

	if (pos.y <= _mapYSize - 2)
	{
		// 아래가 OBSTACLE이고 오른쪽아래가 NONE
		if (GetMapValue(pos + dir[df_DIR_D]) == OBSTACLE && GetMapValue(pos + dir[df_DIR_RD]) != OBSTACLE)
		{
			ret |= 1 << df_DIR_RD;
			ret |= 1 << df_DIR_R;
		}
	}

	return ret;
}

int JumpPointSearch::CheckDown(Pos pos)
{
	int ret = 0;

	if (pos.y == _mapYSize - 1)
	{
		return ret;
	}

	if (pos.x >= 1)
	{
		// 왼쪽이 obstacle이고 왼쪽아래가 NONE
		if (GetMapValue(pos + dir[df_DIR_L]) == OBSTACLE && GetMapValue(pos + dir[df_DIR_LD]) != OBSTACLE)
		{
			ret |= 1 << df_DIR_LD;
			ret |= 1 << df_DIR_D;
		}
	}

	if (pos.x <= _mapXSize - 2)
	{
		// 오른쪽이 obstacle이고 오른쪽아래가 NONE
		if (GetMapValue(pos + dir[df_DIR_R]) == OBSTACLE && GetMapValue(pos + dir[df_DIR_RD]) != OBSTACLE)
		{
			ret |= 1 << df_DIR_RD;
			ret |= 1 << df_DIR_D;
		}

	}

	return ret;
}

int JumpPointSearch::CheckLeft(Pos pos)
{
	int ret = false;

	if (pos.x == 0)
	{
		return ret;
	}

	if (pos.y >= 1)
	{
		// 위쪽이 OBSTACLE, 왼쪽위가 NONE
		if (GetMapValue(pos + dir[df_DIR_U]) == OBSTACLE && GetMapValue(pos + dir[df_DIR_LU]) != OBSTACLE)
		{
			ret |= 1 << df_DIR_LU;
			ret |= 1 << df_DIR_L;

		}
	}

	if (pos.y <= _mapYSize - 2)
	{
		// 아래쪽 OBSTACLE, 왼쪽아래가 NONE
		if (GetMapValue(pos + dir[df_DIR_D]) == OBSTACLE && GetMapValue(pos + dir[df_DIR_LD]) != OBSTACLE)
		{
			ret |= 1 << df_DIR_LD;
			ret |= 1 << df_DIR_L;
		}
	}
	return ret;
}

int JumpPointSearch::CheckUpRight(Pos pos)
{
	int ret = 0;

	if (pos.x <= _mapXSize - 2)
	{
		// 아래가 OBSTACLE이고 오른쪽아래 NONE
		if (GetMapValue(pos + dir[df_DIR_D]) == OBSTACLE && GetMapValue(pos + dir[df_DIR_RD]) != OBSTACLE)
		{
			ret |= 1 << df_DIR_RD;
			ret |= 1 << df_DIR_UR;
		}
	}

	if (pos.y >= 1)
	{
		// 왼쪾이 OBSTACLE이고 왼쪽위가 NONE
		if (GetMapValue(pos + dir[df_DIR_L]) == OBSTACLE && GetMapValue(pos + dir[df_DIR_LU]) != OBSTACLE)
		{
			ret |= 1 << df_DIR_LU;
			ret |= 1 << df_DIR_UR;
		}

	}

	return ret;
}

int JumpPointSearch::CheckRightDown(Pos pos)
{
	int ret = 0;

	if (pos.x <= _mapXSize - 2)
	{
		// 위쪽이 OBSTACLE이고 위오른쪽이 NONE
		if (GetMapValue(pos + dir[df_DIR_U]) == OBSTACLE && GetMapValue(pos + dir[df_DIR_UR]) != OBSTACLE)
		{
			ret |= 1 << df_DIR_UR;
			ret |= 1 << df_DIR_RD;

		}
	}

	if (pos.y >= 1)
	{
		// 왼쪽이 OBSTACLE이고 왼쪽아래가 NONE
		if (GetMapValue(pos + dir[df_DIR_L]) == OBSTACLE && GetMapValue(pos + dir[df_DIR_LD]) != OBSTACLE)
		{
			ret |= 1 << df_DIR_LD;
			ret |= 1 << df_DIR_RD;
		}
	}

	return ret;
}

int JumpPointSearch::CheckLeftDown(Pos pos)
{
	int ret = false;
	if (pos.x >= 1)
	{
		// 위쪽이 OBSTACLE이고 위왼쪽이 NONE
		if (GetMapValue(pos + dir[df_DIR_U]) == OBSTACLE && GetMapValue(pos + dir[df_DIR_LU]) != OBSTACLE)
		{
			ret |= 1 << df_DIR_LU;
			ret |= 1 << df_DIR_LD;

		}
	}


	if (pos.y <= _mapYSize - 2)
	{
		// 오른쪽이 OBSTACLE이고 오른쪽 아래가 NONE
		if (GetMapValue(pos + dir[df_DIR_R]) == OBSTACLE && GetMapValue(pos + dir[df_DIR_RD]) != OBSTACLE)
		{
			ret |= 1 << df_DIR_RD;
			ret |= 1 << df_DIR_LD;
		}
	}
	return ret;
}

int JumpPointSearch::CheckLeftUp(Pos pos)
{
	int ret = 0;

	if (pos.y >= 1)
	{
		// 오른쪽이 OBSTACLE이고 오른쪽위가 NONE
		if (GetMapValue(pos + dir[df_DIR_R]) == OBSTACLE && GetMapValue(pos + dir[df_DIR_UR]) != OBSTACLE)
		{
			ret |= 1 << df_DIR_UR;
			ret |= 1 << df_DIR_LU;
		}
	}

	if (pos.x >= 1)
	{
		// 아래가 OBSTACLE이고 왼쪽아래가 NONE
		if (GetMapValue(pos + dir[df_DIR_L]) == OBSTACLE && GetMapValue(pos + dir[df_DIR_LD]) != OBSTACLE)
		{
			ret |= 1 << df_DIR_LD;
			ret |= 1 << df_DIR_LU;
		}

	}

	return ret;
}

bool JumpPointSearch::CheckAndMakeCorner(Node* p, Pos pos, int direction, uint32 findRange)
{
	Pos originPos;
	originPos = pos;
	//if (pos.y < 0 || pos.x < 0 || pos.x >= _mapXSize || pos.y >= _mapYSize)
	//{
	//	__debugbreak();
	//}

	//if (GetMapValue(pos) == OBSTACLE)
	//{
	//	__debugbreak();
	//}

	//if (GetMapValue(pos) == END)
	//{
	//	__debugbreak();
	//}
	switch (direction)
	{

	case df_DIR_U:
	{
		int distance = 0;
		for(int i = 0; i < findRange; i++)
		{
			pos = pos + dir[df_DIR_U];
			distance += 10;
			if (pos.y < 0)
			{
				break;
			}

			if (GetMapValue(pos) == OBSTACLE)
			{
				break;
			}

			if (GetMapValue(pos) == CLOSE)
			{
				continue;
			}

			if (pos == _dest)
			{
				CreateNode(pos, p, 0, distance);
				return true;
			}

			//SetMap(pos, _checkValue);
			int dirVal = CheckUp(pos);
			if (!dirVal)
			{
				continue;
			}
			else
			{
				CreateNode(pos, p, dirVal, distance);
				break;
			}
		}
	}
	break;

	case df_DIR_R:
	{

		int distance = 0;
		for (int i = 0; i < findRange; i++)
		{
			pos = pos + dir[df_DIR_R];
			distance += 10;

			if (pos.x >= _mapXSize)
			{
				break;
			}


			if (GetMapValue(pos) == OBSTACLE)
			{
				break;
			}

			if (GetMapValue(pos) == CLOSE)
			{
				continue;
			}

			if (pos == _dest)
			{
				CreateNode(pos, p, 0, distance);
				return true;
			}


			//SetMap(pos, _checkValue);

			int dirVal = CheckRight(pos);
			if (!dirVal)
			{
				continue;
			}
			else
			{
				CreateNode(pos, p, dirVal, distance);
				break;
			}
		}
	}
	break;

	case df_DIR_D:
	{

		int distance = 0;
		for (int i = 0; i < findRange; i++)
		{
			pos = pos + dir[df_DIR_D];
			distance += 10;



			if (pos.y >= _mapYSize)
			{
				break;
			}

			if (GetMapValue(pos) == OBSTACLE)
			{
				break;
			}
			if (GetMapValue(pos) == CLOSE)
			{
				continue;
			}


			if (pos == _dest)
			{
				CreateNode(pos, p, 0, distance);
				return true;
			}

			//SetMap(pos, _checkValue);


			int dirVal = CheckDown(pos);
			if (!dirVal)
			{
				continue;
			}
			else
			{
				CreateNode(pos, p, dirVal, distance);
				break;
			}
		}
	}
	break;

	case df_DIR_L:
	{

		int distance = 0;
		for (int i = 0; i < findRange; i++)
		{
			pos = pos + dir[df_DIR_L];
			distance += 10;

			if (pos.x < 0)
			{
				break;
			}

			if (GetMapValue(pos) == OBSTACLE)
			{
				break;
			}

			if (GetMapValue(pos) == CLOSE)
			{
				continue;
			}


			if (pos == _dest)
			{
				CreateNode(pos, p, 0, distance);
				return true;
			}

			//SetMap(pos, _checkValue);


			int dirVal = CheckLeft(pos);
			if (!dirVal)
			{
				continue;
			}
			else
			{
				CreateNode(pos, p, dirVal, distance);
				break;

			}
		}
	}
	break;


	case df_DIR_UR:
	{
		int distance = 0;
		for (int i = 0; i < findRange; i++)
		{
			pos = pos + dir[df_DIR_UR];

			distance += 14;

			if (pos.x >= _mapXSize || pos.y < 0)
			{
				break;
			}

			if (GetMapValue(pos) == OBSTACLE)
			{
				break;
			}

			if (GetMapValue(pos) == CLOSE)
			{
				continue;
			}

			if (pos == _dest)
			{
				CreateNode(pos, p, 0, distance);
				return true;
			}


			//SetMap(pos, _checkValue);


			int dirVal = CheckUpRight(pos);

			// node 만들어져있는가
			Node* nodeHere = nullptr;
			if (dirVal != 0)
			{
				nodeHere = CreateNode(pos, p, dirVal, distance);
			}



			//pos는 기존위치이고
			{
				Pos toUpPos = pos;
				int toUpDistance = 0;
				/*****************************************************
				*
				*                 위 쪽 진 행 로 직
				*
				* ****************************************************/
				for (int i = 0; i < findRange; i++)
				{
					toUpPos = toUpPos + dir[df_DIR_U];

					toUpDistance += 10;
					if (toUpPos.y < 0)
					{
						break;
					}

					if (GetMapValue(toUpPos) == OBSTACLE)
					{
						break;
					}

					if (GetMapValue(toUpPos) == CLOSE)
					{
						continue;
					}


					if (toUpPos == _dest)
					{
						if (nodeHere == nullptr)
						{
							nodeHere = CreateNode(pos, p, 1 << df_DIR_UR, distance);
						}
						// 종료지점 노드 만들기
						CreateNode(toUpPos, nodeHere, 0, toUpDistance);
						return true;
					}

					//SetMap(toUpPos, _checkValue);
					int upDirVal = CheckUp(toUpPos);

					if (upDirVal != 0)
					{
						if (nodeHere == nullptr)
						{
							nodeHere = CreateNode(pos, p, 1 << df_DIR_UR, distance);
						}

						CreateNode(toUpPos, nodeHere, upDirVal, toUpDistance);
						break;

					}
				}
				// 위쪽 진행 로직 끝
			}



			{
				Pos toRightPos = pos;
				int toRightDistance = 0;
				/*****************************************************
				*
				*                 오른쪽 진 행 로 직
				*
				* ****************************************************/
				for (int i = 0; i < findRange; i++)
				{
					toRightPos = toRightPos + dir[df_DIR_R];

					toRightDistance += 10;
					if (toRightPos.x >= _mapXSize)
					{
						break;
					}

					if (GetMapValue(toRightPos) == OBSTACLE)
					{
						break;
					}

					if (GetMapValue(toRightPos) == CLOSE)
					{
						continue;
					}



					if (toRightPos == _dest)
					{
						if (nodeHere == nullptr)
						{
							nodeHere = CreateNode(pos, p, 1 << df_DIR_UR, distance);
						}
						CreateNode(toRightPos, nodeHere, 0, toRightDistance);
						return true;
					}

					//SetMap(toRightPos, _checkValue);

					int rightDirVal = CheckRight(toRightPos);

					if (rightDirVal != 0)
					{
						if (nodeHere == nullptr)
						{
							nodeHere = CreateNode(pos, p, 1 << df_DIR_UR, distance);
						}

						CreateNode(toRightPos, nodeHere, rightDirVal, toRightDistance);
						break;
					}
				}
				// 오른쪽 진행 로직 끝
			}

			if (nodeHere != nullptr)
			{
				break;
			}
		}
	}
	break;

	case df_DIR_RD:
	{
		int distance = 0;
		for (int i = 0; i < findRange; i++)
		{
			pos = pos + dir[df_DIR_RD];

			distance += 14;
			if (pos.x >= _mapXSize || pos.y >= _mapYSize)
			{

				break;
			}

			if (GetMapValue(pos) == OBSTACLE)
			{
				break;
			}

			if (GetMapValue(pos) == CLOSE)
			{
				continue;
			}

			if (pos == _dest)
			{
				CreateNode(pos, p, 0, distance);
				return true;
			}

			//SetMap(pos, _checkValue);

			int dirVal = CheckRightDown(pos);


			// node 만들어져있는가
			Node* nodeHere = nullptr;
			if (dirVal != 0)
			{
				nodeHere = CreateNode(pos, p, dirVal, distance);
			}
			//checkValue가 0이아니었으면 node를 이좌표에 만든상황인데?


			//pos는 기존위치이고
			{
				Pos toDownPos = pos;
				int toDownDistance = 0;
				/*****************************************************
				*
				*                 아래 쪽 진 행 로 직
				*
				* ****************************************************/
				for (int i = 0; i < findRange; i++)
				{
					toDownPos = toDownPos + dir[df_DIR_D];

					toDownDistance += 10;
					if (toDownPos.y >= _mapYSize)
					{
						break;
					}

					if (GetMapValue(toDownPos) == OBSTACLE)
					{
						break;
					}


					if (GetMapValue(toDownPos) == CLOSE)
					{
						continue;
					}

					if (toDownPos == _dest)
					{

						if (nodeHere == nullptr)
						{
							nodeHere = CreateNode(pos, p, 1 << df_DIR_RD, distance);
						}
						// 종료지점 노드 만들기
						CreateNode(toDownPos, nodeHere, 0, toDownDistance);
						return true;
					}

					//SetMap(toDownPos, _checkValue);


					int downDirVal = CheckDown(toDownPos);

					if (downDirVal != 0)
					{
						if (nodeHere == nullptr)
						{
							nodeHere = CreateNode(pos, p, 1 << df_DIR_RD, distance);
						}

						CreateNode(toDownPos, nodeHere, downDirVal, toDownDistance);
						break;
					}
				}
				// 위쪽 진행 로직 끝
			}




			{
				Pos toRightPos = pos;
				int toRightDistance = 0;
				/*****************************************************
				*
				*                 오른쪽 진 행 로 직
				*
				* ****************************************************/
				for (int i = 0; i < findRange; i++)
				{
					// 현재좌표로 부터 오른쪽으로 진행
					toRightPos = toRightPos + dir[df_DIR_R];
					toRightDistance += 10;
					if (toRightPos.x >= _mapXSize)
					{
						break;
					}

					if (GetMapValue(toRightPos) == OBSTACLE)
					{
						break;
					}


					if (GetMapValue(toRightPos) == CLOSE)
					{
						continue;
					}


					if (toRightPos == _dest)
					{
						if (nodeHere == nullptr)
						{
							nodeHere = CreateNode(pos, p, 1 << df_DIR_RD, distance);
						}
						// 종료지점 노드 만들기
						CreateNode(toRightPos, nodeHere, 0, toRightDistance);
						return true;
					}

					//SetMap(toRightPos, _checkValue);

					int rightDirVal = CheckRight(toRightPos);

					if (rightDirVal != 0)
					{
						if (nodeHere == nullptr)
						{
							nodeHere = CreateNode(pos, p, 1 << df_DIR_RD, distance);
						}
						CreateNode(toRightPos, nodeHere, rightDirVal, toRightDistance);
						break;

					}
				}
				// 오른쪽 진행 로직 끝
			}
			if (nodeHere != nullptr)
			{
				break;
			}
		}
	}
	break;

	// 왼쪽 아래
	case df_DIR_LD:
	{

		int distance = 0;
		for (int i = 0; i < findRange; i++)
		{
			pos = pos + dir[df_DIR_LD];
			distance += 14;
			if (pos.x < 0 || pos.y >= _mapYSize)
			{
				break;
			}

			if (GetMapValue(pos) == OBSTACLE)
			{
				break;
			}

			if (GetMapValue(pos) == CLOSE)
			{
				continue;
			}


			if (pos == _dest)
			{
				CreateNode(pos, p, 0, distance);
				return true;
			}

			//SetMap(pos, _checkValue);

			int dirVal = CheckLeftDown(pos);

			// node 만들어져있는가
			Node* nodeHere = nullptr;
			if (dirVal != 0)
			{
				nodeHere = CreateNode(pos, p, dirVal, distance);
			}
			//checkValue가 0이아니었으면 node를 이좌표에 만든상황인데?


			//pos는 기존위치이고
			{
				Pos toDownPos = pos;
				int toDownDistance = 0;
				/*****************************************************
				*
				*                 아래 쪽 진 행 로 직
				*
				* ****************************************************/
				for (int i = 0; i < findRange; i++)
				{
					toDownPos = toDownPos + dir[df_DIR_D];
					// 아래쪽진행로직인데
					toDownDistance += 10;
					if (toDownPos.y >= _mapYSize)
					{
						break;
					}

					if (GetMapValue(toDownPos) == OBSTACLE)
					{
						break;
					}

					if (GetMapValue(toDownPos) == CLOSE)
					{
						continue;
					}

					if (toDownPos == _dest)
					{
						if (nodeHere == nullptr)
						{
							nodeHere = CreateNode(pos, p, 1 << df_DIR_LD, distance);
						}
						// 종료지점 노드 만들기
						CreateNode(toDownPos, nodeHere, 0, toDownDistance);
						return true;
					}

					//SetMap(toDownPos, _checkValue);


					int downDirVal = CheckDown(toDownPos);

					if (downDirVal != 0)
					{

						if (nodeHere == nullptr)
						{
							nodeHere = CreateNode(pos, p, 1 << df_DIR_LD, distance);
						}
						CreateNode(toDownPos, nodeHere, downDirVal, toDownDistance);
						break;
					}
				}
				// 아래쪽 진행 로직 끝
			}




			{
				Pos toLeftPos = pos;
				int toLeftDistance = 0;
				/*****************************************************
				*
				*                 왼쪽  진 행 로 직
				*
				* ****************************************************/
				for (int i = 0; i < findRange; i++)
				{
					toLeftPos = toLeftPos + dir[df_DIR_L];

					toLeftDistance += 10;
					if (toLeftPos.x < 0)
					{
						break;
					}

					if (GetMapValue(toLeftPos) == OBSTACLE)
					{
						break;
					}

					if (GetMapValue(toLeftPos) == CLOSE)
					{
						continue;
					}



					if (toLeftPos == _dest)
					{
						if (nodeHere == nullptr)
						{
							nodeHere = CreateNode(pos, p, 1 << df_DIR_LD, distance);
						}
						// 종료지점 노드 만들기
						CreateNode(toLeftPos, nodeHere, 0, toLeftDistance);
						return true;
					}

					//SetMap(toLeftPos, _checkValue);

					int leftDirVal = CheckLeft(toLeftPos);

					if (leftDirVal != 0)
					{

						if (nodeHere == nullptr)
						{
							nodeHere = CreateNode(pos, p, 1 << df_DIR_LD, distance);
						}
						CreateNode(toLeftPos, nodeHere, leftDirVal, toLeftDistance);
						break;
					}
				}
				//왼쪽 진행로직 끝
			}
			if (nodeHere != nullptr)
			{
				break;
			}
		}
	}
	break;

	case df_DIR_LU:
	{

		int distance = 0;
		for (int i = 0; i < findRange; i++)
		{
			pos = pos + dir[df_DIR_LU];
			distance += 14;
			if (pos.x < 0 || pos.y < 0)
			{
				break;
			}

			if (GetMapValue(pos) == OBSTACLE)
			{
				break;
			}


			if (GetMapValue(pos) == CLOSE)
			{
				continue;
			}

			if (pos == _dest)
			{
				CreateNode(pos, p, 0, distance);
				return true;
			}

			//SetMap(pos, _checkValue);
			int dirVal = CheckLeftUp(pos);

			Node* nodeHere = nullptr;
			if (dirVal != 0)
			{
				nodeHere = CreateNode(pos, p, dirVal, distance);
			}
			//checkValue가 0이아니었으면 node를 이좌표에 만든상황인데?


			//pos는 기존위치이고
			{
				Pos toUpPos = pos;
				int toUpDistance = 0;
				/*****************************************************
				*
				*                위 쪽 진 행 로 직
				*
				* ****************************************************/
				for (int i = 0; i < findRange; i++)
				{
					toUpPos = toUpPos + dir[df_DIR_U];
					// 아래쪽진행로직인데
					toUpDistance += 10;
					if (toUpPos.y < 0)
					{
						break;
					}

					if (GetMapValue(toUpPos) == OBSTACLE)
					{
						break;
					}

					if (GetMapValue(toUpPos) == CLOSE)
					{
						continue;
					}
					//SetMap(toUpPos, _checkValue);


					if (toUpPos == _dest)
					{
						if (nodeHere == nullptr)
						{
							nodeHere = CreateNode(pos, p, 1 << df_DIR_LU, distance);
						}
						// 종료지점 노드 만들기
						CreateNode(toUpPos, nodeHere, 0, toUpDistance);
						return true;
					}

					int upDirVal = CheckUp(toUpPos);

					if (upDirVal != 0)
					{
						if (nodeHere == nullptr)
						{
							nodeHere = CreateNode(pos, p, 1 << df_DIR_LU, distance);
						}
						CreateNode(toUpPos, nodeHere, upDirVal, toUpDistance);
						break;
					}
				}
				// 아래쪽 진행 로직 끝
			}




			{
				Pos toLeftPos = pos;
				int toLeftDistance = 0;
				/*****************************************************
				*
				*                 왼쪽  진 행 로 직
				*
				* ****************************************************/
				for (int i = 0; i < findRange; i++)
				{
					toLeftPos = toLeftPos + dir[df_DIR_L];

					toLeftDistance += 10;
					if (toLeftPos.x < 0)
					{

						break;
					}

					if (GetMapValue(toLeftPos) == OBSTACLE)
					{
						break;
					}

					if (GetMapValue(toLeftPos) == CLOSE)
					{
						continue;
					}

					//SetMap(toLeftPos, _checkValue);

					if (toLeftPos == _dest)
					{
						if (nodeHere == nullptr)
						{
							nodeHere = CreateNode(pos, p, 1 << df_DIR_LU, distance);
						}
						// 종료지점 노드 만들기
						CreateNode(toLeftPos, nodeHere, 0, toLeftDistance);
						return true;

					}

					int leftDirVal = CheckLeft(toLeftPos);

					if (leftDirVal != 0)
					{
						if (nodeHere == nullptr)
						{
							nodeHere = CreateNode(pos, p, 1 << df_DIR_LU, distance);
						}
						CreateNode(toLeftPos, nodeHere, leftDirVal, toLeftDistance);
						break;
					}
				}
				//왼쪽 진행로직 끝
			}
			if (nodeHere != nullptr)
			{
				break;
			}
		}
	}
	break;
	}
	return false;
}

Node* JumpPointSearch::CreateNode(Pos pos, Node* parent, int direction, int distanceFromParent)
{
	// 클로즈인 곳 생성 -> return
	if (GetMapValue(pos) == CLOSE)
	{
		return nullptr;
	}
	else {
		//open인곳 생성 -> 패런트비교해서 바꾸고 return
		if (GetMapValue(pos) == OPEN)
		{
			auto it = openList.begin();
			for (; it != openList.end(); ++it)
			{
				if ((*it)->_pos.x == pos.x && (*it)->_pos.y == pos.y)
				{
					break;
				}
			}
			if (it == openList.end())
			{
				__debugbreak();
			}
			CheckAndChangeParent(parent, (*it), distanceFromParent);
			//ReopenNode((*it));
			return *it;
		}
	}

	Node* n = new Node;
	n->_pos = pos;
	n->_g = parent->_g + distanceFromParent;
	int h = GetH(pos, _dest);
	n->_h = h;
	n->_f = n->_g + h;
	n->_parent = parent;

	for (int i = 0; i < 8; i++)
	{
		if ((1 << i) & direction)
			n->AddDirection(i);

	}
	//_debugNodeList[pos] = n;
	openList.push_back(n);
	SetMap(pos, OPEN);
	return n;
}

void JumpPointSearch::CheckAndChangeParent(Node* newParent, Node* exist, int distance)
{
	if (newParent->_g + distance < exist->_g)
	{
		//변경할 필요있으면 변경하고
		exist->_parent = newParent;
		exist->_g = newParent->_g + distance;
		exist->_f = exist->_g + exist->_h;
	}
}

int JumpPointSearch::GetH(Pos& pos, Pos& dest)
{
	return abs(dest.y - pos.y) * 10 + abs(dest.x - pos.x) * 10;
}

void JumpPointSearch::FindShortestPath(vector<Pos>& path, vector<Pos>& result)
{
	if (path.size() < 2)
	{
		result = path;
		return;
	}

	//시작노드넣으면안되지
	//result.push_back(path[0]);

	int pathSize = (int)path.size();
	int nodeFromIndex = 0;
	int nodeToIndex = nodeFromIndex + 2;

	while (nodeToIndex < pathSize)
	{
		
		Pos nodeFromPos = path[nodeFromIndex];
		Pos nodeToPos = path[nodeToIndex];

		if (!CalculateBresenham(nodeFromPos, nodeToPos))
		{
			// 장애물 있으면
			uint32 distance = abs(nodeFromPos.y - nodeToPos.y) + abs(nodeFromPos.x - nodeToPos.x);
			if (distance > OMIT_RANGE)
			{
				result.push_back(path[nodeToIndex - 1]);
			}

			nodeFromIndex = nodeToIndex - 1;
			nodeToIndex = nodeFromIndex + 2;
		}
		else {
			// 장애물 없으면
			nodeToIndex++;
		}
	}

	//마지막 노드넣고
	result.push_back(path[pathSize - 1]);

	//// 최단 경로 디버깅용 벡터
	//for (auto it = result.rbegin(); it != result.rend(); ++it)
	//{
	//	_debugShortestPath.push_back(*it);
	//}
}

bool JumpPointSearch::CalculateBresenham(Pos start, Pos end)
{
	int x1 = ToWorld(start.x);
	int y1 = ToWorld(start.y);
	int x2 = ToWorld(end.x);
	int y2 = ToWorld(end.y);


	// 여기는 그냥 start랑 dest사이의 노드들 _bresenhamNode에 넣고
	int dx = abs(x2 - x1);
	int dy = abs(y2 - y1);

	int sx = (x1 < x2) ? 1 : -1;
	int sy = (y1 < y2) ? 1 : -1;

	int cumul = 0;

	if (dx >= dy)
	{
		while (true)
		{
			if (_fineMap[y1][x1] == OBSTACLE)
				return false;
		
			// 현재 노드
			if (x1 == x2 && y1 == y2)
			{
				break;
			}
			x1 += sx;
			cumul += dy;
			int cumul2 = 2 * cumul;

			if (cumul2 > dx)
			{
				y1 += sy;
				cumul -= dx;
			}
		}
	}
	else {
		while (true)
		{
			if (_fineMap[y1][x1] == OBSTACLE)
				return false;
			
			// 현재 노드
			if (x1 == x2 && y1 == y2)
			{
				break;
			}
			y1 += sy;
			cumul += dx;
			int cumul2 = 2 * cumul;

			if (cumul2 > dy)
			{
				x1 += sx;
				cumul -= dy;
			}
		}
	}

	return true;
}

void JumpPointSearch::ClearNode()
{
	for (auto it = openList.begin(); it != openList.end(); ++it)
	{
		delete* it;
	}
	openList.clear();

	for (auto it = closeList.begin(); it != closeList.end(); ++it)
	{
		delete* it;
	}
	closeList.clear();
}

