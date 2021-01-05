#pragma once
#include <stdlib.h>
#include <map>

typedef unsigned short USHORT;
struct MemoryBlock;


//全局内存池为单例类。使用的时候只需要一个实例即可，并不需要多个实例。
class MemoryPool
{
private:
	static USHORT poolMapIndex;
	//分配不同内存块时其对应的映射表
	std::map<int, int>poolMap;
	//内存池对齐大小。
	const int POOLALIGNMENT = 8;
	//初始化内存块
	int initBlockCount;
	//内存块不足增长的块数。
	int growBlockcount;
	//首子内存池下标
	unsigned firstIndex;
	//末内存池下标。
	unsigned lastIndex;
	//最多16中不同内存块大小，也就是说子内存池最多有16个。
	MemoryBlock* memoryHashMap[16];
	MemoryBlock** mpPtr;
	//计算不同内存块对应的hashCode
	int Hash(int);
	//对齐字节
	int AlignBytes(int);
	//返回分配块大小。
	int GetUnitSize(int);
protected:
	static MemoryPool* memoryPoolInstance;
	MemoryPool(int initBlockSize = 1024, int growBlockSize = 256);
public:
	//分配内存
	void* Alloc(int);
	//释放内存。
	void FreeAlloc(void*);
	//返回全局内存池实例
	static MemoryPool* GetInstance();
	~MemoryPool();
};
/*
MemoryPool* mp = MemoryPool::GetInstance();
class TestMemoryPool
{
private:
    int value1 = 10;
    int value2 = 20;
    float val = 5.9f;
public:
    TestMemoryPool(){}
    static void* operator new(size_t)
    {
        return mp->Alloc(sizeof(TestMemoryPool));
    }
    static void operator delete(void* p) {
        mp->FreeAlloc(p);
    }
};

class BigClassTest
{
private:
    char bc[500];
    int v = 10;
    char* p = nullptr;
public:
    BigClassTest() {
        //memset(bc, 0, sizeof(bc));
    }
    static void* operator new(size_t)
    {
        return mp->Alloc(sizeof(BigClassTest));
    }
    static void operator delete(void* p) {
        mp->FreeAlloc(p);
    }
};
class ThirdMemoryTest
{
private:
    int v[100];
    std::string s;
public:
    ThirdMemoryTest()
    {
        //memset(v, 0, sizeof(v));
        s = "";
    }
    static void* operator new(size_t)
    {
        return mp->Alloc(sizeof(ThirdMemoryTest));
    }
    static void operator delete(void* p)
    {
        mp->FreeAlloc(p);
    }
};

void test()
{
    TestMemoryPool* v = nullptr;
    for (int i = 0; i < 2050; i++)
    {
        v = new TestMemoryPool();
    }
    printf("testing\n");

    BigClassTest* b = nullptr;
    for (int i = 0; i < 3000;i++)
    {
        b = new BigClassTest();
    }

    ThirdMemoryTest* t = nullptr;
    for (int j = 0; j < 1920; j++)
    {
        t = new ThirdMemoryTest();
    }

}

int main()
{
    test();
    int p = 12;
    int*q = &p;
    short v = p;
    ListPool<int>iList;
    iList.AddNode(10);
    for (int i = 98; i > 1; i /= 10)
    {
        iList.AddNode(i);
    }

    printf("List size %u\n", iList.ListSize());
}*/