#pragma once
#include <new.h>
#include <stdlib.h>
#include <stack>
#include "Type.h"


struct TlsLog
{
	int threadId;
	int size;
	int mallocCount;
};

struct ObjectPoolUsageMonitor {
	int size;
	int mallocCount;
};

//extern std::map<int, ObjectPoolUsageMonitor> _objectPoolMonitor;
extern int releaseStackSize;
#define MAX_TLS_LOG_INDEX 40
#define DEFAULT_MAX_POOL_SIZE 500

#define DEFAULT_RELEASE_NUM 100
extern TlsLog _objectPoolMonitor[MAX_TLS_LOG_INDEX];




template <class DATA, bool bPlacementNew>
class TlsObjectPool;


template <class DATA>
class TlsObjectPool<DATA, true> // bPlacement가 flase일때의 특수화
{
	struct st_BLOCK_NODE
	{
		DATA data;
		st_BLOCK_NODE* next = nullptr;
	};

public:
	TlsObjectPool()
	{
		//printf("TlsObjectPool()\n");
		InitializeSRWLock(&_releasedDataLock);
		_maxPoolSize = DEFAULT_MAX_POOL_SIZE;
		_releaseNum = DEFAULT_RELEASE_NUM;
		_PoolSizeToRelease = _maxPoolSize + _releaseNum;
	}

	TlsObjectPool(int blockNum, int releaseNum)
	{
		InitializeSRWLock(&_releasedDataLock);
		//InitializeSRWLock(&_releasedDataLock);
		_maxPoolSize = blockNum;
		_initialBlockNum = blockNum;

		//TODO: iBlockNum만큼 Alloc
		if (blockNum == 0)
		{
			_maxPoolSize = DEFAULT_MAX_POOL_SIZE;
			// TODO: NOTHING
		}

		if (releaseNum == 0)
		{
			if (_maxPoolSize < 500)
			{
				//500개보다 작으면 그냥 100개
				_releaseNum = DEFAULT_RELEASE_NUM;
			}
			else {
				//아니면 20%
				_releaseNum = _maxPoolSize / 5;
			}
		}
		else {
			_releaseNum = releaseNum;
		}

		_PoolSizeToRelease = _maxPoolSize + _releaseNum;
	}

	int GetSize()
	{
		return _size;
	}

	void Init()
	{
		if (_initialBlockNum == 0)
		{
			//printf("init %d", GetCurrentThreadId());
			_size = 0;
		}
		else {
			//printf("init %d", GetCurrentThreadId());
			_freeNode = new st_BLOCK_NODE;
			_freeNode->next = nullptr;
			int i = 0;
			for (; i < _initialBlockNum - 1; i++)
			{
				st_BLOCK_NODE* node = new st_BLOCK_NODE;
				node->next = _freeNode;
				_freeNode = node;
			}
			//_toReleaseNode = _freeNode;s
			_size = _initialBlockNum;
			_mallocNumPerPool = _initialBlockNum;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// 블럭 하나를 할당받는다.
	//
	// Parameters: 없음.
	// Return: (DATA *) 데이타 블럭 포인터.
	//////////////////////////////////////////////////////////////////////////
	DATA* Alloc(void)
	{
		if (_size == -1)
		{
			Init();
		}
		int currentThreadId = GetCurrentThreadId();
		_objectPoolMonitor[currentThreadId] = { _size, _mallocNumPerPool };
		/*st_BLOCK_NODE* freeNode = _freeNode;*/
		if (_freeNode != nullptr)
		{
			//있는것 가져다줫으면
			DATA* ret = new(&_freeNode->data) DATA;
			_freeNode = _freeNode->next;
			_size--;
			return ret;
		}
		else
		{

			int releasedDataStackSize = -1;
			st_BLOCK_NODE* node = nullptr;
			AcquireSRWLockExclusive(&_releasedDataLock);
			releasedDataStackSize = _releasedData.size();
			if (!_releasedData.empty())
			{
				// 비어있지 않으며
				node = _releasedData.top();
				_releasedData.pop();
				_size += _releaseNum;
			}
			ReleaseSRWLockExclusive(&_releasedDataLock);

			if (node == nullptr)
			{
				//printf("malloc, stack Size : %d\n", releasedDataStackSize);
				// stack에도 없으면
				node = new st_BLOCK_NODE;
				_mallocNumPerPool++;
				if (_freeNode != nullptr)
				{
					__debugbreak();
				}
				node->next = _freeNode; // _freeNode == nullptr
				return &(node->data);
			}
			else {
				//stack에서 _releaseNum만큼 뺴왓으면
				_freeNode = node;
				DATA* ret = &_freeNode->data;
				_freeNode = _freeNode->next;
				_size--;
				return ret;
			}
			// 새로 생성했으면 _capacity++;
		}
	}

	//void Free(DATA* pData)
	//{
	//	if (!_bInit)
	//	{
	//		//Init
	//		_bInit = true;
	//		Init();
	//	}

	//	//_size++;
	//	int size = ++_size;
	//	if (size <= _maxPoolSize)
	//	{
	//		((st_BLOCK_NODE*)pData)->next = _freeNode;

	//		_freeNode = (st_BLOCK_NODE*)pData;
	//		return;
	//	}
	//}


	void	Free(DATA* pData)
	{
		int size = ++_size;
		int currentThreadId = GetCurrentThreadId();
		pData->~DATA();
		_objectPoolMonitor[currentThreadId] = { _size, _mallocNumPerPool };
		if (size == 0)
		{
			Init();
		}
		//_size++;
		if (size <= _maxPoolSize)
		{
			((st_BLOCK_NODE*)pData)->next = _freeNode;
			_freeNode = (st_BLOCK_NODE*)pData;
			return;
		}
		else if (size == _maxPoolSize + 1)
		{
			((st_BLOCK_NODE*)pData)->next = _freeNode;

			/*if (_freeNode == nullptr)
			{
				__debugbreak();
			}*/
			//이게 연결끊어야할거 next nullptr로 만들아야할것
			_toDisconnectNode = (st_BLOCK_NODE*)pData;
			//이게 stack에 넣을것
			_freeNodeAfterRelease = _freeNode;
			_freeNode = (st_BLOCK_NODE*)pData;

			return;
		}
		else {
			((st_BLOCK_NODE*)pData)->next = _freeNode;

			_freeNode = (st_BLOCK_NODE*)pData;
			if (size == _PoolSizeToRelease)
			{
				/*if (_freeNode == nullptr)
				{
					__debugbreak();
				}*/

				//연결 끊고
				_toDisconnectNode->next = nullptr;
				AcquireSRWLockExclusive(&_releasedDataLock);
				releaseStackSize = _releasedData.size();
				_releasedData.push((st_BLOCK_NODE*)pData);
				ReleaseSRWLockExclusive(&_releasedDataLock);

				_freeNode = _freeNodeAfterRelease;
				_size -= _releaseNum;
				//_toReleaseNode = _freeNode;
				return;
			}
		}

		// 사용중인것
		//return true;
	}


public:
	//각 thread마다 갖고있어야하는거
	inline static thread_local st_BLOCK_NODE* _freeNode = nullptr; // 애초에 freeNode가 뭐지? 처음으로 줄 수 있는 것

	inline static thread_local int _size = -1; // 현재 풀에 남아있는 개수

	//int _capacity = 0; // 현재 확보 된 블럭 갯수 ( 메모리풀 내부의 전체 갯수 )
	inline static thread_local st_BLOCK_NODE* _freeNodeAfterRelease = nullptr;
	//st_BLOCK_NODE* _toReleaseNode = nullptr;
	inline static thread_local st_BLOCK_NODE* _toDisconnectNode = nullptr;


public:
	//TLS 풀 공용
	SRWLOCK _releasedDataLock;
	std::stack<st_BLOCK_NODE*> _releasedData;
	// int _capacity = 0; // 현재 확보 된 블럭 갯수 ( 메모리풀 내부의 전체 갯수 ) 멀티쓰레드 풀에서 capacity가 필요없음
	// 현재 이풀에 존재하는 갯수랑, 몇개가 외부에서 사용중인지가 중요한데
	// 몇개가 외부에서 사용중인지도 중요하지않네
	// 다른풀로 들어가면 모르잖아

	// 풀의 개수를 추적하려면 뭐가 중요할까?
	// 이풀에 남아있는 사이즈랑
	// 건네준 총갯수가 중요하지 않을까
	// 건네준 총갯수도 필요없고


	// 결론
	// 그냥 이풀에 남아있는 사이즈랑
	// 새로 생성한 갯수만 있으면
	// 프로세스에서 전체 몇개만큼의 노드가 돌아다니는지 알수있겠네


	//int _useCount = 0; //  현재 외부에서 사용중인 블럭갯수? // 이걸 버리자// 필요하면 _capacity - _size니까
public:
	//CObjectPool<DATA, false> _tlOp;
	inline static thread_local int _mallocNumPerPool = 0;
	int _maxPoolSize = 0;
	int _PoolSizeToRelease = 0;
	int _releaseNum = 0;
	int _initialBlockNum = 0;
};

template <class DATA>
class TlsObjectPool<DATA, false> // bPlacement가 flase일때의 특수화
{
	struct st_BLOCK_NODE
	{
		DATA data;
		st_BLOCK_NODE* next = nullptr;
	};

public:
	TlsObjectPool()
	{
		//printf("TlsObjectPool()\n");
		InitializeSRWLock(&_releasedDataLock);
		_maxPoolSize = DEFAULT_MAX_POOL_SIZE;
		_releaseNum = DEFAULT_RELEASE_NUM;
		_PoolSizeToRelease = _maxPoolSize + _releaseNum;
	}

	TlsObjectPool(int blockNum, int releaseNum)
	{
		InitializeSRWLock(&_releasedDataLock);
		//InitializeSRWLock(&_releasedDataLock);
		_maxPoolSize = blockNum;
		_initialBlockNum = blockNum;

		//TODO: iBlockNum만큼 Alloc
		if (blockNum == 0)
		{
			_maxPoolSize = DEFAULT_MAX_POOL_SIZE;
			// TODO: NOTHING
		}

		if (releaseNum == 0)
		{
			if (_maxPoolSize < 500)
			{
				//500개보다 작으면 그냥 100개
				_releaseNum = DEFAULT_RELEASE_NUM;
			}
			else {
				//아니면 20%
				_releaseNum = _maxPoolSize / 5;
			}
		}
		else {
			_releaseNum = releaseNum;
		}

		_PoolSizeToRelease = _maxPoolSize + _releaseNum;
	}

	int GetSize()
	{
		return _size;
	}

	void Init()
	{
		if (_initialBlockNum == 0)
		{
			//printf("init %d", GetCurrentThreadId());
			_size = 0;
		}
		else {
			//printf("init %d", GetCurrentThreadId());
			_freeNode = new st_BLOCK_NODE;
			_freeNode->next = nullptr;
			int i = 0;
			for (; i < _initialBlockNum - 1; i++)
			{
				st_BLOCK_NODE* node = new st_BLOCK_NODE;
				node->next = _freeNode;
				_freeNode = node;
			}
			//_toReleaseNode = _freeNode;s
			_size = _initialBlockNum;
			_mallocNumPerPool = _initialBlockNum;
		}

		_threadId = GetCurrentThreadId();
		_tlsLogIndex = InterlockedIncrement(&g_tlsLogIndex);
	}

	//////////////////////////////////////////////////////////////////////////
	// 블럭 하나를 할당받는다.
	//
	// Parameters: 없음.
	// Return: (DATA *) 데이타 블럭 포인터.
	//////////////////////////////////////////////////////////////////////////
	DATA* Alloc(void)
	{
		if (_size == -1)
		{
			Init();
		}
		InterlockedIncrement64(&_totalUseCount);
		
		//int currentThreadId = GetCurrentThreadId();
		_objectPoolMonitor[_tlsLogIndex] = { _threadId, _size, _mallocNumPerPool };
		/*st_BLOCK_NODE* freeNode = _freeNode;*/
		if (_freeNode != nullptr)
		{
			//있는것 가져다줫으면
			DATA* ret = &_freeNode->data; 
				_freeNode = _freeNode->next;
			_size--;
			return ret;
		}
		else
		{

			int releasedDataStackSize = -1;
			st_BLOCK_NODE* node = nullptr;
			AcquireSRWLockExclusive(&_releasedDataLock);
			releasedDataStackSize = _releasedData.size();
			if (!_releasedData.empty())
			{
				// 비어있지 않으며
				node = _releasedData.top();
				_releasedData.pop();
				_size += _releaseNum;
			}
			ReleaseSRWLockExclusive(&_releasedDataLock);

			if (node == nullptr)
			{
				//printf("malloc, stack Size : %d\n", releasedDataStackSize);
				// stack에도 없으면
				node = new st_BLOCK_NODE;
				_mallocNumPerPool++;
				if (_freeNode != nullptr)
				{
					__debugbreak();
				}
				node->next = _freeNode; // _freeNode == nullptr
				return &(node->data);
			}
			else {
				//stack에서 _releaseNum만큼 뺴왓으면
				_freeNode = node;
				DATA* ret = &_freeNode->data;
				_freeNode = _freeNode->next;
				_size--;
				return ret;
			}
			// 새로 생성했으면 _capacity++;
		}
	}

	void	Free(DATA* pData)
	{
		int size = ++_size;
		//int currentThreadId = GetCurrentThreadId();
		if (size == 0)
		{
			Init();
		}
		_objectPoolMonitor[_tlsLogIndex] = { _threadId, _size, _mallocNumPerPool };
		InterlockedDecrement64(&_totalUseCount);
		//_size++;
		if (size <= _maxPoolSize)
		{
			((st_BLOCK_NODE*)pData)->next = _freeNode;
			_freeNode = (st_BLOCK_NODE*)pData;
			return;
		}
		else if (size == _maxPoolSize + 1)
		{
			((st_BLOCK_NODE*)pData)->next = _freeNode;

			/*if (_freeNode == nullptr)
			{
				__debugbreak();
			}*/
			//이게 연결끊어야할거 next nullptr로 만들아야할것
			_toDisconnectNode = (st_BLOCK_NODE*)pData;
			//이게 stack에 넣을것
			_freeNodeAfterRelease = _freeNode;
			_freeNode = (st_BLOCK_NODE*)pData;

			return;
		}
		else {
			((st_BLOCK_NODE*)pData)->next = _freeNode;

			_freeNode = (st_BLOCK_NODE*)pData;
			if (size == _PoolSizeToRelease)
			{
				/*if (_freeNode == nullptr)
				{
					__debugbreak();
				}*/

				//연결 끊고
				_toDisconnectNode->next = nullptr;
				AcquireSRWLockExclusive(&_releasedDataLock);
				releaseStackSize = _releasedData.size();
				_releasedData.push((st_BLOCK_NODE*)pData);
				ReleaseSRWLockExclusive(&_releasedDataLock);

				_freeNode = _freeNodeAfterRelease;
				_size -= _releaseNum;
				//_toReleaseNode = _freeNode;
				return;
			}
		}

		// 사용중인것
		//return true;
	}


public:
	//각 thread마다 갖고있어야하는거
	inline static thread_local st_BLOCK_NODE* _freeNode = nullptr; // 애초에 freeNode가 뭐지? 처음으로 줄 수 있는 것

	inline static thread_local int _size = -1; // 현재 풀에 남아있는 개수

	//int _capacity = 0; // 현재 확보 된 블럭 갯수 ( 메모리풀 내부의 전체 갯수 )
	inline static thread_local st_BLOCK_NODE* _freeNodeAfterRelease = nullptr;
	//st_BLOCK_NODE* _toReleaseNode = nullptr;
	inline static thread_local st_BLOCK_NODE* _toDisconnectNode = nullptr;


public:
	//TLS 풀 공용
	SRWLOCK _releasedDataLock;
	std::stack<st_BLOCK_NODE*> _releasedData;
	// int _capacity = 0; // 현재 확보 된 블럭 갯수 ( 메모리풀 내부의 전체 갯수 ) 멀티쓰레드 풀에서 capacity가 필요없음
	// 현재 이풀에 존재하는 갯수랑, 몇개가 외부에서 사용중인지가 중요한데
	// 몇개가 외부에서 사용중인지도 중요하지않네
	// 다른풀로 들어가면 모르잖아

	// 풀의 개수를 추적하려면 뭐가 중요할까?
	// 이풀에 남아있는 사이즈랑
	// 건네준 총갯수가 중요하지 않을까
	// 건네준 총갯수도 필요없고


	// 결론
	// 그냥 이풀에 남아있는 사이즈랑
	// 새로 생성한 갯수만 있으면
	// 프로세스에서 전체 몇개만큼의 노드가 돌아다니는지 알수있겠네


	//int _useCount = 0; //  현재 외부에서 사용중인 블럭갯수? // 이걸 버리자// 필요하면 _capacity - _size니까
public:
	//CObjectPool<DATA, false> _tlOp;
	inline static thread_local int _mallocNumPerPool = 0;
	int _maxPoolSize = 0;
	int _PoolSizeToRelease = 0;
	int _releaseNum = 0;
	int _initialBlockNum = 0;

	inline static int64 _totalUseCount = 0;

	//로그용
	inline static thread_local int _threadId = 0;

	inline static thread_local int _tlsLogIndex = 0;
	long g_tlsLogIndex = -1;
};



