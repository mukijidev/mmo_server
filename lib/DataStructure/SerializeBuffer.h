#pragma once
#include "Type.h"
#include <stdlib.h>
#include "LockFreeObjectPool.h"
#include "TlsObjectPool.h"
#include "PacketHeader.h"



#define TLS_POOL
class CPacket
{
	friend CPacket& operator<<(CPacket& packet, FVector& vec);
	friend CPacket& operator>>(CPacket& packet, FVector& vec);
	friend CPacket& operator>>(CPacket& packet, FRotator& rot);
	friend CPacket& operator<<(CPacket& packet, FRotator& rot);
	friend CPacket& operator<<(CPacket& packet, PlayerInfo& info);
	friend CPacket& operator>>(CPacket& packet, PlayerInfo& info);
	friend CPacket& operator<<(CPacket& packet, MonsterInfo& info);
	friend CPacket& operator>>(CPacket& packet, MonsterInfo& info);
	friend CPacket& operator>>(CPacket& packet, Pos& pos);
	friend CPacket& operator<<(CPacket& packet, Pos& pos);


	friend class TlsObjectPool<CPacket, false>;
	friend class LockFreeObjectPool<CPacket, false>;

	/*---------------------------------------------------------------
	Packet Enum.

	----------------------------------------------------------------*/
	enum en_PACKET
	{
		BUFFER_DEFAULT_SIZE = 256		// 패킷의 기본 버퍼 사이즈.
	};

	//////////////////////////////////////////////////////////////////////////
	// 생성자, 파괴자.
	//
	// Return:
	//////////////////////////////////////////////////////////////////////////

public:
	CPacket()
	{
		_buffer = (char*)malloc(sizeof(char) * BUFFER_DEFAULT_SIZE);
		if (_buffer == nullptr)
		{
			__debugbreak();
		}
		_bufferSize = BUFFER_DEFAULT_SIZE;
	}

	CPacket(int bufferSize)
	{
		_buffer = (char*)malloc(sizeof(char) * bufferSize);
		if (_buffer == nullptr)
		{
			__debugbreak();
		}
		_bufferSize = bufferSize;
	}

	~CPacket()
	{
		free(_buffer);
	}

public:

	//////////////////////////////////////////////////////////////////////////
	// 패킷 청소.
	//
	// Parameters: 없음.
	// Return: 없음.
	//////////////////////////////////////////////////////////////////////////
	void Clear(void)
	{
		_refCount = 0;
		_writePos = 0;
		_readPos = 0;
		_dataSize = 0;
		_bEncoded = false;
	}

	//////////////////////////////////////////////////////////////////////////
	// 버퍼 사이즈 얻기.
	//
	// Parameters: 없음.
	// Return: (int)패킷 버퍼 사이즈 얻기.
	//////////////////////////////////////////////////////////////////////////
	int	GetBufferSize(void)
	{
		return _bufferSize;
	}
	//////////////////////////////////////////////////////////////////////////
	// 현재 사용중인 사이즈 얻기.
	//
	// Parameters: 없음.
	// Return: (int)사용중인 데이타 사이즈.
	//////////////////////////////////////////////////////////////////////////
	int	GetDataSize(void)
	{
		return _dataSize;
	}

	//////////////////////////////////////////////////////////////////////////
	// 버퍼 포인터 얻기.
	//
	// Parameters: 없음.
	// Return: (char *)버퍼 포인터.
	//////////////////////////////////////////////////////////////////////////
	char* GetBufferPtr(void)
	{
		return _buffer;
	}

	//////////////////////////////////////////////////////////////////////////
	// 버퍼 Pos 이동. (음수이동은 안됨)
	// GetBufferPtr 함수를 이용하여 외부에서 강제로 버퍼 내용을 수정할 경우 사용.
	//
	// Parameters: (int) 이동 사이즈.
	// Return: (int) 이동된 사이즈.
	//////////////////////////////////////////////////////////////////////////
	int		MoveWritePos(int size)
	{
		//TODO : 범위체크
		_writePos += size;
		_dataSize += size;
		return size;
	}

	int		MoveReadPos(int size)
	{
		// TODO:
		// 밤위체크
		_readPos += size;
		_dataSize -= size;
		return size;
	}

	/* ============================================================================= */
	// 연산자 오버로딩
	/* ============================================================================= */
	CPacket& operator = (CPacket& srcPacket)
	{
		// 버퍼 복사하고
		// read pos, write pos 복사해야하는건가
		if (_buffer != nullptr)
		{
			free(_buffer);
		}
		// TODO: 한번더 보기
		// 흠 뭐 잘못한거 있으려나
		_buffer = (char*)malloc(sizeof(char) * srcPacket._bufferSize);
		if (_buffer == nullptr)
		{
			__debugbreak();
		}

		memcpy(_buffer, srcPacket._buffer, srcPacket._bufferSize);

		_dataSize = srcPacket._dataSize;
		_bufferSize = srcPacket._bufferSize;
		_writePos = srcPacket._writePos;
		_readPos = srcPacket._readPos;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	// 넣기.	각 변수 타입마다 모두 만듬.
	//////////////////////////////////////////////////////////////////////////

	// 넣기 TODO: writePos 증가, 버퍼에 복사
	CPacket& operator << (unsigned char byValue)
	{
		*((unsigned char*)(&_buffer[_writePos])) = byValue;
		//(*_buffer) = byValue;
		_writePos += sizeof(unsigned char);

		_dataSize += sizeof(unsigned char);
		return *this;
	}

	CPacket& operator << (char chValue)
	{
		*((char*)(&_buffer[_writePos])) = chValue;
		_writePos += sizeof(char);

		_dataSize += sizeof(char);
		return *this;
	}

	CPacket& operator << (short shValue)
	{
		*((short*)(&_buffer[_writePos])) = shValue;
		_writePos += sizeof(short);

		_dataSize += sizeof(short);
		return *this;
	}

	CPacket& operator << (unsigned short wValue)
	{
		*((unsigned short*)(&_buffer[_writePos])) = wValue;
		_writePos += sizeof(unsigned short);

		_dataSize += sizeof(unsigned short);
		return *this;
	}

	CPacket& operator << (int iValue)
	{
		*((int*)(&_buffer[_writePos])) = iValue;
		_writePos += sizeof(int);

		_dataSize += sizeof(int);
		return *this;
	}

	CPacket& operator << (long lValue)
	{
		*((long*)(&_buffer[_writePos])) = lValue;
		_writePos += sizeof(long);

		_dataSize += sizeof(long);
		return *this;
	}

	CPacket& operator <<(uint32 dwValue)
	{
		*((uint32*)(&_buffer[_writePos])) = dwValue;
		_writePos += sizeof(uint32);

		_dataSize += sizeof(uint32);
		return *this;
	}

	CPacket& operator << (float fValue)
	{
		*((float*)(&_buffer[_writePos])) = fValue;
		_writePos += sizeof(float);

		_dataSize += sizeof(float);
		return *this;
	}

	CPacket& operator << (__int64 iValue)
	{
		*((__int64*)(&_buffer[_writePos])) = iValue;
		_writePos += sizeof(__int64);

		_dataSize += sizeof(__int64);
		return *this;
	}

	CPacket& operator << (double dValue)
	{
		*((double*)(&_buffer[_writePos])) = dValue;
		_writePos += sizeof(double);

		_dataSize += sizeof(double);
		return *this;
	}

	//CPacket& operator << (FVector& vValue)
	//{
	//	memcpy(&_buffer[_writePos], &vValue, sizeof(double) * 3);
	//	_writePos += sizeof(FVector);
	//	_dataSize += sizeof(FVector);
	//	return *this;
	//}

	
	CPacket& operator << (WCHAR* wCharValue)
	{

		for (int i = 0; i <= wcslen(wCharValue); i++)
		{
			*((WCHAR*)(&_buffer[_writePos])) = wCharValue[i];
			_writePos += sizeof(WCHAR);
			_dataSize += sizeof(WCHAR);
		}
		return *this;
	}

	


	//////////////////////////////////////////////////////////////////////////
	// 빼기.	각 변수 타입마다 모두 만듬.
	//////////////////////////////////////////////////////////////////////////

	// 빼기 TODO: readPos 증가 src에 복사
	CPacket& operator >> (BYTE& byValue)
	{
		byValue = *(BYTE*)(&_buffer[_readPos]);
		_readPos += sizeof(BYTE);

		_dataSize -= sizeof(BYTE);
		return *this;
	}

	CPacket& operator >> (char& chValue)
	{
		chValue = *(char*)(&_buffer[_readPos]);
		_readPos += sizeof(char);

		_dataSize -= sizeof(char);
		return *this;
	}

	CPacket& operator >> (short& shValue)
	{
		shValue = *(short*)(&_buffer[_readPos]);
		_readPos += sizeof(short);

		_dataSize -= sizeof(shValue);
		return *this;
	}

	CPacket& operator >> (WORD& wValue)
	{
		wValue = *(WORD*)(&_buffer[_readPos]);
		_readPos += sizeof(WORD);

		_dataSize -= sizeof(WORD);
		return *this;
	}

	CPacket& operator >> (int& iValue)
	{
		iValue = *(int*)(&_buffer[_readPos]);
		_readPos += sizeof(int);

		_dataSize -= sizeof(int);
		return *this;
	}

	CPacket& operator >> (uint32& dwValue)
	{
		dwValue = *(uint32*)(&_buffer[_readPos]);
		_readPos += sizeof(uint32);

		_dataSize -= sizeof(uint32);
		return *this;
	}

	CPacket& operator >> (float& fValue)
	{
		fValue = *(float*)(&_buffer[_readPos]);
		_readPos += sizeof(float);

		_dataSize -= sizeof(float);
		return *this;
	}

	CPacket& operator >> (__int64& iValue)
	{
		iValue = *(__int64*)(&_buffer[_readPos]);
		_readPos += sizeof(__int64);

		_dataSize -= sizeof(__int64);
		return *this;
	}

	CPacket& operator >> (double& dValue)
	{
		dValue = *(double*)(&_buffer[_readPos]);
		_readPos += sizeof(double);

		_dataSize -= sizeof(double);
		return *this;
	}



	CPacket& operator >> (WCHAR* wCharValue)
	{
		int i = 0;
		while (true)
		{
			wCharValue[i] = *((WCHAR*)(&_buffer[_readPos]));
			i++;
			if (*((WCHAR*)(&_buffer[_readPos])) == L'\0')
			{
				_readPos += sizeof(WCHAR);
				_dataSize -= sizeof(WCHAR);
				break;
			}
			_readPos += sizeof(WCHAR);
			_dataSize -= sizeof(WCHAR);
		}
		return *this;
	}
	//////////////////////////////////////////////////////////////////////////
	// 데이타 얻기.
	//
	// Parameters: (char *)Dest 포인터. (int)Size.
	// Return: (int)복사한 사이즈.
	//////////////////////////////////////////////////////////////////////////
	int	GetData(char* dest, int size)
	{
		//TODO: 에러차이
		// 흠 readPos 옮겨야하는건가 옮겨야하네
		// header읽을떄 쓸것같은데
		memcpy(dest, _buffer + _readPos, size);
		_readPos += size;

		_dataSize -= size;
		return size;
	}

	//////////////////////////////////////////////////////////////////////////
	// 데이타 삽입.
	//
	// Parameters: (char *)Src 포인터. (int)SrcSize.
	// Return: (int)복사한 사이즈.
	//////////////////////////////////////////////////////////////////////////
	int	PutData(char* src, int size)
	{
		memcpy(_buffer + _writePos, src, size);
		_writePos += size;

		_dataSize += size;
		return size;
	}

	void IncRefCount(int inc)
	{
		InterlockedAdd64(&_refCount, inc);
	}

	void IncRefCount()
	{
		InterlockedIncrement64(&_refCount);
	}

	void DecRefCount()
	{
		InterlockedDecrement64(&_refCount);
	}

public:
	static CPacket* Alloc()
	{
		CPacket* packet = _packetPool.Alloc();
		packet->Clear();
		packet->_refCount = 1;
		return packet;
	}

	

	static void Free(CPacket* packet)
	{
		//wprintf(L"free packet : %llx\n", (void*)packet);
		if (InterlockedDecrement64(&packet->_refCount) == 0)
		{
			_packetPool.Free(packet);
		}
		/*int64 ret = InterlockedDecrement64(& packet->_refCount);
		if (ret == 0)
		{
			_packetPool.Free(packet);
		}*/

		//if (ret <= 0)
		//{
		//	if (ret == -1)
		//	{
		//		__debugbreak();
		//	}
		//	if (packet->_bufferSize != BUFFER_DEFAULT_SIZE)
		//	{
		//		packet->Resize(BUFFER_DEFAULT_SIZE);
		//	}

		//	_packetPool.Free(packet);
		//	//Free(packet)
		//}
	}

	static int64 GetUseCount()
	{
		return _packetPool._totalUseCount;
	}

	/*static void Free(CPacket* packet)
	{
		_packetPool.Free(packet);
	}*/

public:
	

	void Encode(uint16 packetKey)
	{
		if (_bEncoded)
		{
			return;
		}
		char* buff = _buffer;
		_bEncoded = true;
		NetHeader* header = (NetHeader*)buff;

		// 진짜 패킷에서는 앞에 len하고 패킷코드
		//xor하고 다시 xor하면 기존데이터가되는데
		// d1 ^ ( RK + 1 ) = p1 
		// p1 ^ (k + 1  ) = e1; 

		// 여기서 e1에서 어떻게 d1으로 바꾸지
		// e1 e2
		// d1 d2
		uint16 len = header->_len;
		uint8 randomKey = header->_randKey;
		uint8 checkSum = 0;

		int iEnd = len + sizeof(NetHeader);


		for (int i = sizeof(NetHeader); i < iEnd; i++)
		{
			checkSum += buff[i];
		}
		checkSum %= 256;

		//header->_checkSum = checkSum;
		//e2를 d2로 바꾸려면 e1이 필요하네
		int p = checkSum ^ (randomKey + 1);
		header->_checkSum = p ^ (packetKey + 1);
		int prevData = header->_checkSum;
		//int e;


		for (int i = sizeof(NetHeader); i < iEnd; i++)
		{
			int add = (i - sizeof(NetHeader) + 2);

			char& c = buff[i];
			p = c ^ (p + randomKey + add);
			c = p ^ (prevData + packetKey + add);
			prevData = c;
			//TODO:
			// d1 > e1;
			// d2 > e2;
			// d3 > e3;
			// d4 > e4;
		}

	}

	bool Decode(NetHeader* header, uint16 packetKkey)
 	{
		int size = sizeof(NetHeader);
		uint8 p;
		uint8 randKey = header->_randKey;
		uint8 encodedChecksum = header->_checkSum;

		uint8 pPrev = encodedChecksum ^ (packetKkey + 1); // p1
		uint8 prevData = encodedChecksum;


		uint8 decodedCheckSum = pPrev ^ (randKey + 1); // d1

		//todo : validate checksum
		uint8 checkSum = 0;
		for (int i = 0; i < header->_len; i++)
		{
			char& c = _buffer[i];
			p = c ^ (prevData + packetKkey + (i + 2));
			prevData = c;

			c = p ^ (pPrev + randKey + (i + 2));
			checkSum += c;
			pPrev = p;
			//TODO:
			// e1 > d1
			// e2 > d2
			// e3 > d3
			// e4 > d4
		}

		/*for (int i = 0; i < header->_len; i++)
		{
			checkSum += _buffer[i];
		}*/
		checkSum %= 256;

		if (decodedCheckSum != checkSum)
		{

			return false;
		}
		return true;
	}

	char* GetReadPtr()
	{
		return _buffer + _readPos;
	}
		
public:
	//Monitor 때문에 만드는건데
	void Resize(int size)
	{
		free(_buffer);
		_buffer = (char*)malloc(sizeof(char) * size);
		_bufferSize = size;
	}



	


private:
	int64 _refCount = 0;
	char* _buffer;
	//int64 _refCount = 0;
	//
	int	_bufferSize;
	bool _bEncoded = false;
	//------------------------------------------------------------
	// 현재 버퍼에 사용중인 사이즈.
	//------------------------------------------------------------
	int	_dataSize = 0;
	//------------------------------------------------------------
	//
	//------------------------------------------------------------
	int _writePos = 0;
	int _readPos = 0;


#ifdef TLS_POOL
	static inline TlsObjectPool<CPacket, false> _packetPool{ 5000, 4000 };
#else
	static inline procademy::CObjectPool<CPacket, false> _packetPool;

#endif

};

