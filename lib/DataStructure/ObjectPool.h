#pragma once
#include <new.h>
#include <stdlib.h>



template <class DATA, bool bPlacementNew>
class CObjectPool;

// bPlacementNew가 true일 때의 특수화
template <class DATA>
class CObjectPool<DATA, true>
{

	struct st_BLOCK_NODE
	{
		DATA data;
		st_BLOCK_NODE* next;
	};

public:
	//////////////////////////////////////////////////////////////////////////
	// 생성자, 파괴자.
	//
	// Parameters:	(int) 초기 블럭 개수.
	//				(bool) Alloc 시 생성자 / Free 시 파괴자 호출 여부
	// Return:
	//////////////////////////////////////////////////////////////////////////
	CObjectPool()
	{
		// Alloc시 생성자 / Free시 파괴자 호출 여부

	}

	CObjectPool(int blockNum)
	{
		//TODO: iBlockNum만큼 Alloc
		if (blockNum == 0)
		{
			//TODO: NOTHING
		}
		else {
			_freeNode = (st_BLOCK_NODE*)malloc(sizeof(st_BLOCK_NODE));
			_freeNode->next = nullptr;
			int i = 0;
			for (; i < blockNum - 1; i++)
			{
				st_BLOCK_NODE* node = (st_BLOCK_NODE*)malloc(sizeof(st_BLOCK_NODE));
				node->next = _freeNode;
				_freeNode = node;
			}
		}

		_capacity += blockNum;
	}
	virtual	~CObjectPool()
	{
		//TODO: 소멸자에서 할것
		// 1. 방치
		// 2. 풀안에 남아있는것만 매모리 정리
		// 3. 다 추적해서 메모리 정리
	}

	template <typename... Args>
	DATA* Alloc(Args&&... args)
	{
		_useCount++;
		if (_freeNode != nullptr)
		{
			// 줄거 있으면?
			DATA* ret = new(&_freeNode->data) DATA(std::forward<Args>(args)...);
			_freeNode = _freeNode->next;

			return ret;
		}
		else
		{
			// 줄거없으면
			// 새로 만들어야지
			st_BLOCK_NODE* newNode = (st_BLOCK_NODE*)malloc(sizeof(st_BLOCK_NODE));
			DATA* ret = new(&newNode->data) DATA(std::forward<Args>(args)...);
			newNode->next = nullptr;
			_freeNode = nullptr;

			// 새로 만들었을 때만 capacity++
			_capacity++;

			return ret;
		}
	}


	void	Free(DATA* pData)
	{
		_useCount--;
		// 소멸자 호출 하고
		pData->~DATA();
		((st_BLOCK_NODE*)pData)->next = _freeNode;
		_freeNode = (st_BLOCK_NODE*)pData;


		return;
	}


	//////////////////////////////////////////////////////////////////////////
	// 현재 확보 된 블럭 개수를 얻는다. (메모리풀 내부의 전체 개수)
	//
	// Parameters: 없음.
	// Return: (int) 메모리 풀 내부 전체 개수
	//////////////////////////////////////////////////////////////////////////
	int		GetCapacityCount(void) { return _capacity; }

	//////////////////////////////////////////////////////////////////////////
	// 현재 사용중인 블럭 개수를 얻는다.
	//
	// Parameters: 없음.
	// Return: (int) 사용중인 블럭 개수.
	//////////////////////////////////////////////////////////////////////////
	int		GetUseCount(void) {
		return _useCount;
	}


	// 스택 방식으로 반환된 (미사용) 오브젝트 블럭을 관리.
private:
	st_BLOCK_NODE* _freeNode; // 애초에 freeNode가 뭐지? 처음으로 줄 수 있는 것
	int _capacity = 0; // 현재 확보 된 블럭 갯수 ( 메모리풀 내부의 저네 갯수)
	int _useCount = 0; // useCount ?? 현재 사용중인 블럭갯수? 
};


// bPlacementNew가 false일 때의 특수화
template <class DATA>
class CObjectPool<DATA, false>
{

	struct st_BLOCK_NODE
	{
		DATA data;
		st_BLOCK_NODE* next = nullptr;
	};

public:

	//////////////////////////////////////////////////////////////////////////
	// 생성자, 파괴자.
	//
	// Parameters:	(int) 초기 블럭 개수.
	//				(bool) Alloc 시 생성자 / Free 시 파괴자 호출 여부
	// Return:
	//////////////////////////////////////////////////////////////////////////
	CObjectPool()
	{
		// Alloc시 생성자 / Free시 파괴자 호출 여부
	}

	CObjectPool(int blockNum)
	{
		// Alloc시 생성자 / Free 시 파괴자 호출 여부

		//TODO: iBlockNum만큼 Alloc
		if (blockNum == 0)
		{
			// TODO: NOTHING
		}
		else {
			_freeNode = new(st_BLOCK_NODE);
			_freeNode->next = nullptr;
			int i = 0;
			for (; i < blockNum - 1; i++)
			{
				st_BLOCK_NODE* node = new(st_BLOCK_NODE);
				node->next = _freeNode;
				_freeNode = node;
			}

		}
		_capacity += blockNum;
	}

	virtual	~CObjectPool()
	{
		//TODO: 소멸자에서 할것
		// 1. 방치
		// 2. 풀안에 남아있는것만 매모리 정리
		// 3. 다 추적해서 메모리 정리
	}


	DATA* Alloc(void)
	{
		_useCount++;
		if (_freeNode != nullptr)
		{
			DATA* ret = &_freeNode->data;
			_freeNode = _freeNode->next;
			// 사용중인것 
			return ret;
		}
		else
		{
			st_BLOCK_NODE* node = new st_BLOCK_NODE;
			//DATA* ret = &(node->data);
			node->next = _freeNode;
			_capacity++;
			return &(node->data);

		}

	}

	void	Free(DATA* pData)
	{
		_useCount--;
		((st_BLOCK_NODE*)pData)->next = _freeNode;
		_freeNode = (st_BLOCK_NODE*)pData;
	}


	//////////////////////////////////////////////////////////////////////////
	// 현재 확보 된 블럭 개수를 얻는다. (메모리풀 내부의 전체 개수)
	//
	// Parameters: 없음.
	// Return: (int) 메모리 풀 내부 전체 개수
	//////////////////////////////////////////////////////////////////////////
	int		GetCapacityCount(void) { return _capacity; }

	//////////////////////////////////////////////////////////////////////////
	// 현재 사용중인 블럭 개수를 얻는다.
	//
	// Parameters: 없음.
	// Return: (int) 사용중인 블럭 개수.
	//////////////////////////////////////////////////////////////////////////
	int		GetUseCount(void) {
		return _useCount;
	}

	// 스택 방식으로 반환된 (미사용) 오브젝트 블럭을 관리.
private:
	st_BLOCK_NODE* _freeNode; // 애초에 freeNode가 뭐지? 처음으로 줄 수 있는 것
	int _capacity = 0; // 현재 확보 된 블럭 갯수 ( 메모리풀 내부의 저네 갯수)
	int _useCount = 0; // useCount ?? 현재 사용중인 블럭갯수? 
};