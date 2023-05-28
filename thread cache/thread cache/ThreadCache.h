#pragma once
#include "Common.h"

class ThreadCache
{
public:
	// 申请和释放内存对象
	void* Allocate(size_t size)
	{
		assert(size <= MAX_BYTES);
		size_t alignSize = SizeClass::RoundUp(size);
		size_t index = SizeClass::Index(size);

		if (!_freeLists[index].Empty())
		{
			return _freeLists[index].Pop();
		}

		else
		{
			return FetchFromCentralCache(index, alignSize);
		}

	}
	void Deallocate(void* ptr, size_t size);

	// 从中心缓存获取对象
	void* FetchFromCentralCache(size_t index, size_t size);
private:
	FreeList _freeLists[NFREELISTS];
};

// TLS thread local storage
static _declspec(thread) ThreadCache* pTLSThreadCache = nullptr;