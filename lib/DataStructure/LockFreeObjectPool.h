#pragma once
#include <new.h>
#include <stdlib.h>
#include "Type.h"
#include <utility>




	template <class DATA, bool bPlacementNew>
	class LockFreeObjectPool;

	// bPlacementNew가 true일 때의 특수화
	template <class DATA>
	class LockFreeObjectPool<DATA, true>
	{

		struct st_BLOCK_NODE
		{
			DATA data;
			st_BLOCK_NODE* prev;
		};

	public:

		//////////////////////////////////////////////////////////////////////////
		// 생성자, 파괴자.
		//
		// Parameters:	(int) 초기 블럭 개수.
		//				(bool) Alloc 시 생성자 / Free 시 파괴자 호출 여부
		// Return:
		//////////////////////////////////////////////////////////////////////////
		LockFreeObjectPool()
		{
			// Alloc시 생성자 / Free시 파괴자 호출 여부
			_demaskValue = 0b11111111'11111111'1;
			_demaskValue <<= 47;
			_demaskValue = ~_demaskValue;

		}

		LockFreeObjectPool(int blockNum)
		{
			// Alloc시 생성자 / Free 시 파괴자 호출 여부
			_demaskValue = 0b11111111'11111111'1;
			_demaskValue <<= 47;
			_demaskValue = ~_demaskValue;
			//TODO: iBlockNum만큼 Alloc
			if (blockNum == 0)
			{
				// TODO: NOTHING
			}
			else {
				DATA** data = new DATA * [blockNum];
				for (int i = 0; i < blockNum; i++)
				{
					data[i] = Alloc();
				}

				for (int i = 0; i < blockNum; i++)
				{
					Free(data[i]);
				}
				delete[] data;;
			}
			_capacity = blockNum;
		}

		virtual	~LockFreeObjectPool()
		{
			//TODO: 소멸자에서 할것
			// 1. 방치
			// 2. 풀안에 남아있는것만 매모리 정리
			// 3. 다 추적해서 메모리 정리
		}



		template <typename... Args>
		DATA* Alloc(Args&&... args)
		{
			InterlockedIncrement64(&_useCount);
			DATA* ret = nullptr;
			st_BLOCK_NODE* newTop = nullptr;
			int64 oldTop = NULL;
			int64 oldTopDemasked = 0;
			int64 newTopMasked = 0;
			do {
				oldTop = _top;

				// 본인이 그냥 oldTop null로 봤으면 
				//한개 생성해서 던짐
				if (oldTop == NULL)
				{
					InterlockedIncrement64(&_capacity);
					st_BLOCK_NODE* newNode = (st_BLOCK_NODE*)malloc(sizeof(st_BLOCK_NODE));
					newNode->prev = nullptr;
					return new(&(newNode->data)) DATA(std::forward<Args>(args)...);
				}
				oldTopDemasked = (int64)oldTop;
				oldTopDemasked &= _demaskValue;
				ret = &(((st_BLOCK_NODE*)oldTopDemasked)->data);

				newTop = ((st_BLOCK_NODE*)oldTopDemasked)->prev;
				newTopMasked = (int64)newTop;
				if (newTopMasked != 0)
				{
					int64 maskValue = InterlockedIncrement64(&_masking);
					maskValue <<= 47;
					newTopMasked |= maskValue;
				}

				if ((void*)newTopMasked == (void*)oldTop)
				{
					__debugbreak();
				}

			} while (InterlockedCompareExchange64(&_top, newTopMasked, (LONG64)oldTop) != (LONG64)oldTop);

			new(ret) DATA(std::forward<Args>(args)...);
			return ret;
		}

		// push
		void	Free(DATA* pData)
		{
			InterlockedDecrement64(&_useCount);
			int64 maskvalue = InterlockedIncrement64(&_masking);
			maskvalue <<= 47;

			pData->~DATA();
			st_BLOCK_NODE* newNode = (st_BLOCK_NODE*)pData;

			int64 topNode = NULL;
			int64 masked = (int64)newNode;
			masked |= maskvalue;


			do {
				topNode = _top;
				int64 topNodeDemasked = topNode & _demaskValue;

				newNode->prev = (st_BLOCK_NODE*)topNodeDemasked;
				if (masked == topNode)
				{
					__debugbreak();
				}
			} while (InterlockedCompareExchange64(&_top, (LONG64)masked, (LONG64)topNode) != (LONG64)topNode);

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

		}


		// 스택 방식으로 반환된 (미사용) 오브젝트 블럭을 관리.
	private:
		int64 _top = NULL; // 애초에 freeNode가 뭐지? 처음으로 줄 수 있는 것
		int64 _capacity = 0; // 현재 확보 된 블럭 갯수 ( 메모리풀 내부의 저네 갯수)
		int64 _useCount = 0; // useCount ?? 현재 사용중인 블럭갯수? 
		int64 _masking = 0;
		int64 _demaskValue = 0;
	};


	// bPlacementNew가 false일 때의 특수화
	template <class DATA>
	class LockFreeObjectPool<DATA, false>
	{

		struct st_BLOCK_NODE
		{
			DATA data;
			st_BLOCK_NODE* prev;
		};

	public:

		//////////////////////////////////////////////////////////////////////////
		// 생성자, 파괴자.
		//
		// Parameters:	(int) 초기 블럭 개수.
		//				(bool) Alloc 시 생성자 / Free 시 파괴자 호출 여부
		// Return:
		//////////////////////////////////////////////////////////////////////////
		LockFreeObjectPool()
		{
			// Alloc시 생성자 / Free시 파괴자 호출 여부
			_demaskValue = 0b11111111'11111111'1;
			_demaskValue <<= 47;
			_demaskValue = ~_demaskValue;

		}

		LockFreeObjectPool(int blockNum)
		{
			// Alloc시 생성자 / Free 시 파괴자 호출 여부
			_demaskValue = 0b11111111'11111111'1;
			_demaskValue <<= 47;
			_demaskValue = ~_demaskValue;
			//TODO: iBlockNum만큼 Alloc
			if (blockNum == 0)
			{
				// TODO: NOTHING
			}
			else {
				DATA** data = new DATA * [blockNum];
				for (int i = 0; i < blockNum; i++)
				{
					data[i] = Alloc();
				}

				for (int i = 0; i < blockNum; i++)
				{
					Free(data[i]);
				}
				delete[] data;;
			}
			_capacity = blockNum;
		}

		virtual	~LockFreeObjectPool()
		{
			//TODO: 소멸자에서 할것
			// 1. 방치
			// 2. 풀안에 남아있는것만 매모리 정리
			// 3. 다 추적해서 메모리 정리
		}

		DATA* Alloc(void)
		{
			InterlockedIncrement64(&_useCount);
			DATA* ret = nullptr;

			st_BLOCK_NODE* newTop = nullptr;
			int64 oldTop = NULL;
			int64 oldTopDemasked = 0;
			int64 newTopMasked = 0;
			do {
				oldTop = _top;


				// 본인이 그냥 oldTop null로 봤으면 
				//한개 생성해서 던짐
				if (oldTop == NULL)
				{
					InterlockedIncrement64(&_capacity);
					st_BLOCK_NODE* node = new st_BLOCK_NODE;
					node->prev = nullptr;
					return &(node->data);
				}
				oldTopDemasked = (int64)oldTop;
				oldTopDemasked &= _demaskValue;
				ret = &(((st_BLOCK_NODE*)oldTopDemasked)->data);

				newTop = ((st_BLOCK_NODE*)oldTopDemasked)->prev;
				newTopMasked = (int64)newTop;
				if (newTopMasked != 0)
				{
					int64 maskValue = InterlockedIncrement64(&_masking);
					maskValue <<= 47;
					newTopMasked |= maskValue;
				}

				if ((void*)newTopMasked == (void*)oldTop)
				{
					__debugbreak();
				}


			} while (InterlockedCompareExchange64(&_top, newTopMasked, (LONG64)oldTop) != (LONG64)oldTop);

			return ret;
		}


		void	Free(DATA* pData)
		{
			InterlockedDecrement64(&_useCount);
			int64 maskvalue = InterlockedIncrement64(&_masking);
			maskvalue <<= 47;

			st_BLOCK_NODE* newNode = (st_BLOCK_NODE*)pData;

			int64 topNode = NULL;
			int64 masked = (int64)newNode;
			masked |= maskvalue;

			do {
				topNode = _top;
				int64 topNodeDemasked = topNode & _demaskValue;

				newNode->prev = (st_BLOCK_NODE*)topNodeDemasked;
				if (masked == topNode)
				{
					__debugbreak();
				}
			} while (InterlockedCompareExchange64(&_top, (LONG64)masked, (LONG64)topNode) != (LONG64)topNode);
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



	private:
		int64 _top = NULL; // 애초에 freeNode가 뭐지? 처음으로 줄 수 있는 것
		int64 _capacity = 0; // 현재 확보 된 블럭 갯수 ( 메모리풀 내부의 전체 갯수)
		int64 _useCount = 0; // useCount 현재 풀에서 할당받아서 쓰고있는 갯수
		int64 _masking = 0;
		int64 _demaskValue = 0;
	};














