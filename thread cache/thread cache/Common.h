#pragma once
#include<iostream>
#include<vector>
#include<thread>
#include<time.h>
#include<assert.h>
using std::cout;
using std::endl;


//小于等于MAX_BYTES，就找thread cache申请
//大于MAX_BYTES，就直接找page cache或者系统堆申请
static const size_t MAX_BYTES = 256 * 1024;
//thread cache和central cache自由链表哈希桶的表大小
static const size_t NFREELISTS = 208;


static void*& NextObj(void* obj)
{
	return *(void**)obj;
}

//管理切分好的小对象的自由链表
class FreeList
{
public:
	void Push(void* obj)
	{
		assert(obj);

		//头插
		//*(void**)obj = _freeList;
		NextObj(obj) = _freeList;
		_freeList = obj;
	}
	//从自由链表头部获取一个对象
	void* Pop()
	{
		assert(_freeList);

		//头删
		void* obj = _freeList;
		_freeList = NextObj(_freeList);
		return obj;
	}

	bool Empty()
	{
		return _freeList == nullptr;
	}

private:
	void* _freeList = nullptr; //自由链表

};


//管理对齐和映射等关系
class SizeClass
{
public:
	// 整体控制在最多10%左右的内碎片浪费
	// [1,128]					8byte对齐	    freelist[0,16)
	// [128+1,1024]				16byte对齐	    freelist[16,72)
	// [1024+1,8*1024]			128byte对齐	    freelist[72,128)
	// [8*1024+1,64*1024]		1024byte对齐     freelist[128,184)
	// [64*1024+1,256*1024]		8*1024byte对齐   freelist[184,208)
	//获取向上对齐后的字节数
	/*
	* static inline size_t_RoundUp(size_t size, size_t alignNum)
	{
		size_t alignSize = 0;
		if(size % alignNum != 0)
		{
			alignSize = (byte / alignNum + 1)*alignNum;
		}

		else
		{
			alignSize = size;
		}
		return alignSize;
	}
	*/

	//1-8
	static inline size_t _RoundUp(size_t bytes, size_t alignNum)
	{
		return ((bytes + alignNum - 1) & ~(alignNum - 1));
	}

	static inline size_t RoundUp(size_t size)
	{
		if (size <= 128)
		{
			return _RoundUp(size, 8);
		}

		else if (size <= 1024)
		{
			return _RoundUp(size, 16);
		}

		else if (size <= 8 * 1024)
		{
			return _RoundUp(size, 128);
		}

		else if (size <= 64 * 1024)
		{
			return _RoundUp(size, 1024);
		}

		else if (size <= 256 * 1024)
		{
			return _RoundUp(size, 8 * 1024);
		}

		else
		{
			assert(false);
			return -1;
		}
	}

	/*size_t _Index(size_t bytes, size_t alignNum)
	{
		if (bytes % alignNum == 0)
		{
			return bytes / alignNum - 1;
		}

		else
		{
			return bytes / alignNum;
		}
	}*/

	static inline size_t _Index(size_t bytes, size_t align_shift)
	{
		return ((bytes + (1 << align_shift) - 1) >> align_shift) - 1;
	}

	//获取对应哈希桶的下标
	static inline size_t Index(size_t bytes)
	{
		//每个区间有多少个自由链表
		static size_t group_array[4] = { 16,56,56,56 };
		if (bytes <= 128)
		{
			return _Index(bytes, 3);
		}

		else if (bytes <= 1024) {
			return _Index(bytes - 128, 4) + group_array[0];
		}

		else if (bytes <= 8 * 1024) 
		{
			return _Index(bytes - 1024, 7) + group_array[1] + group_array[0];
		}

		else if (bytes <= 64 * 1024)
		{
			return _Index(bytes - 8 * 1024, 10) + group_array[2] + group_array[1] + group_array[0];
		}

		else if (bytes <= 256 * 1024) 
		{
			return _Index(bytes - 64 * 1024, 13) + group_array[3] + group_array[2] + group_array[1] + group_array[0];
		}

		else 
		{
			assert(false);
		}

		return -1;
	}
};
