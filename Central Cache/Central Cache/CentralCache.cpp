#define _CRT_SECURE_NO_WARNINGS 1
#include"CentralCache.h"

CentralCache CentralCache::_sInst;

// 获取一个非空的span
Span* CentralCache::GetOneSpan(SpanList& list, size_t size)
{
	// ...
	return nullptr;
}


//从central cache获取一定数量的对象给thread cache
size_t CentralCache::FetchRangeObj(void*& start, void*& end, size_t n, size_t size)
{
	size_t index = SizeClass::Index(size);
	_spanLists[index]._mtx.lock(); //加锁

	//在对应哈希桶中获取一个非空的span
	Span* span = GetOneSpan(_spanLists[index], size);
	assert(span); //span不为空
	assert(span->_freeList); //span当中的自由链表也不为空

	//从span中获取n个对象
	//如果不够n个，有多少拿多少
	start = span->_freeList;
	end = span->_freeList;
	size_t actualNum = 1;
	while (NextObj(end) && n - 1)
	{
		end = NextObj(end);
		actualNum++;
		n--;
	}
	span->_freeList = NextObj(end); //取完后剩下的对象继续放到自由链表
	NextObj(end) = nullptr; //取出的一段链表的表尾置空
	span->_useCount += actualNum; //更新被分配给thread cache的计数

	_spanLists[index]._mtx.unlock(); //解锁
	return actualNum;
}
