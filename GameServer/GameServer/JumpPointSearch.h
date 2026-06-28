#pragma once
#include "Type.h"
#include <list>
#include <vector>
#include "Type.h"

#define COARSE_CELL 25
#define MAX_PATH_RANGE_CELLS 200
#define MAX_NODES 10000

#define OMIT_RANGE  1


enum CellValue {
	NONE = 0,
	OBSTACLE = 1,
	START = 2,
	END = 3,
	OPEN = 4,
	CLOSE = 5,
	ROUTE = 6,
	BRESENHAM = 10,
	CHECK = 20,
};

struct Node {
	Pos _pos;
	int _f;
	int _g;
	int _h;
	Node* _parent;

	int directionArr[8] = { 0, };

	Node() {
		_pos = Pos{ 0,0 };
		_f = 0;
		_g = 0;
		_h = 0;
		_parent = nullptr;
	}

	Node(Pos pos, int f, int g, int h, Node* p)
	{
		_pos = pos;
		_f = f;
		_g = g;
		_h = h;
		_parent = p;
	}

	void AddDirection(int dir)
	{
		directionArr[dir] = 1;
	}
};

struct NodeComparator {
	bool operator()(const Node* a, const Node* b) const {
		// 우선순위 큐에서 더 작은 값을 가지는 Node를 우선시하도록 비교
		return a->_f < b->_f;
	}
};

class JumpPointSearch
{
	//TODO: 생성자로 맵 받고
public:
	JumpPointSearch(uint8** coarse, int32 cNy, int32 cNx, uint8** fine, int32 fNy, int32 fNx, int cellSize);
	~JumpPointSearch();

public:
	//std::vector<Pos> FindPath(Pos start, Pos end);
	void FindPath(Pos start, Pos end, std::vector<Pos>& path);
	void FindFirstPath(Pos start, Pos end, Pos& firstPath);
	bool CalculateBresenham(Pos start, Pos end);

private:
	Node* CreateStartNode(Pos pos);
	int GetMapValue(Pos pos) {
		return _jpsMap[pos.y][pos.x];
	}
	void SetMap(Pos& pos, uint8 value);
	int CheckUp(Pos pos);
	int CheckRight(Pos pos);
	int CheckDown(Pos pos);
	int CheckLeft(Pos pos);
	int CheckUpRight(Pos pos);
	int CheckRightDown(Pos pos);
	int CheckLeftDown(Pos pos);
	int CheckLeftUp(Pos pos);
	bool CheckAndMakeCorner(Node* p, Pos pos, int direction,uint32 findRange);
	Node* CreateNode(Pos pos, Node* parent, int direction, int distanceFromParent);
	void CheckAndChangeParent(Node* newParent, Node* exist, int distance);
	int GetH(Pos& pos, Pos& dest);
	void FindShortestPath(std::vector<Pos>& path, std::vector<Pos>& reuslt);
	bool LineOfSightFineWorld(int x1, int y1, int x2, int y2);
	void ClearNode();

private:
	uint8** _originMap;
	uint8** _jpsMap;

	int32 _mapYSize;
	int32 _mapXSize;
	
	std::list<Node*> openList;
	std::list<Node*> closeList;
	Pos _start;
	Pos _dest;

	uint8** _fineMap;
	int32 _fineY;
	int32 _fineX;
	int _C;

	inline int ToCoarse(int world) const { return world / _C; }
	inline int ToWorld(int cell) const { return cell * _C + _C / 2; };
};

