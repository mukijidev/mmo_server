#pragma once
#include <stdio.h>
#include "LockFreeObjectPool.h"

template<typename T>
class LockFreeStack
{
public:
	struct Node
	{
		T _data;
		int64 _prev;
	};

	LockFreeStack()
	{
		_demaskValue = 0b11111111'11111111'1;
		_demaskValue <<= 47;
		_demaskValue = ~_demaskValue;
	}

	~LockFreeStack()
	{
		for (int i = 0; i < _size; i++)
		{
			T a;
			Pop(a);
		}
	}

	void Push(T data)
	{
		Node* newNode = objectPool.Alloc();
		newNode->_data = data;

		int64 maskvalue = InterlockedIncrement64(&_masking);
		maskvalue <<= 47;

		int64 masked = (int64)newNode;
		masked |= maskvalue;

		int64 topNode = 0; // 내가 바라보고 있는 top

		do {
			/*InterlockedIncrement64(&_masking);*/

			topNode = _top;
			int64 topNodeDemasked = topNode & _demaskValue;

			newNode->_prev = topNodeDemasked; // 새노드의 prev를 탑으로 만들었고

			if (masked == topNode)
			{
				__debugbreak();
			}
		} while (InterlockedCompareExchange64(&_top, masked, topNode) != topNode);

		return;
	}

	bool Pop(T& out) {
		int64 newTop = 0;
		int64 oldTop = NULL;
		int64 oldTopDemasked = 0;
		int64 newTopMasked = 0;
		T ret;
		do {
			oldTop = _top;

			if (oldTop == NULL)
			{
				return false;
			}

			oldTopDemasked = (int64)oldTop;
			oldTopDemasked &= _demaskValue;
			ret = ((Node*)oldTopDemasked)->_data;

			newTop = ((Node*)oldTopDemasked)->_prev;

			if (newTop != 0)
			{
				int64 maskValue = InterlockedIncrement64(&_masking);
				maskValue <<= 47;
				newTopMasked = newTop;
				newTopMasked |= maskValue;
			}


		} while (InterlockedCompareExchange64(&_top, newTopMasked, oldTop) != oldTop);

		objectPool.Free((Node*)oldTopDemasked);
		out = ret;

		return true;
	}

	int size()
	{
		return _size;
	}

	int64 GetObjectPoolCapacity()
	{
		return objectPool.GetCapacityCount();
	}

private:
	LockFreeObjectPool<Node, false> objectPool;
	int64 _top = NULL;
	long _size = 0;
	int64 _masking = 0;
	int64 _demaskValue = 0;
};