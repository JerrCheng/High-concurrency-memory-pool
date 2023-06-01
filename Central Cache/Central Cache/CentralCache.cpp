#define _CRT_SECURE_NO_WARNINGS 1
#include"CentralCache.h"

CentralCache CentralCache::_sInst;

// ��ȡһ���ǿյ�span
Span* CentralCache::GetOneSpan(SpanList& list, size_t size)
{
	// ...
	return nullptr;
}


//��central cache��ȡһ�������Ķ����thread cache
size_t CentralCache::FetchRangeObj(void*& start, void*& end, size_t n, size_t size)
{
	size_t index = SizeClass::Index(size);
	_spanLists[index]._mtx.lock(); //����

	//�ڶ�Ӧ��ϣͰ�л�ȡһ���ǿյ�span
	Span* span = GetOneSpan(_spanLists[index], size);
	assert(span); //span��Ϊ��
	assert(span->_freeList); //span���е���������Ҳ��Ϊ��

	//��span�л�ȡn������
	//�������n�����ж����ö���
	start = span->_freeList;
	end = span->_freeList;
	size_t actualNum = 1;
	while (NextObj(end) && n - 1)
	{
		end = NextObj(end);
		actualNum++;
		n--;
	}
	span->_freeList = NextObj(end); //ȡ���ʣ�µĶ�������ŵ���������
	NextObj(end) = nullptr; //ȡ����һ������ı�β�ÿ�
	span->_useCount += actualNum; //���±������thread cache�ļ���

	_spanLists[index]._mtx.unlock(); //����
	return actualNum;
}
