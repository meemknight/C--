//////////////////////////////////////////////////
//freeListAllocator.h				beta 0.3
//Copyright(c) 2020 Luta Vlad
//https://github.com/meemknight/freeListAllocator
//////////////////////////////////////////////////

#pragma once
#include <Windows.h>
#include <mutex>

#undef min
#undef max

#define KB(x) (x) * 1024ull
#define MB(x) KB((x)) * 1024ull
#define GB(x) MB((x)) * 1024ull

///set this to 1 if you want to link to new/delete
///also set the heap size,
#define LINK_TO_GLOBAL_ALLOCATOR 0
#define DEFAULT_ASSERT_FUNC 1
#define HEAP_SIZE MB(10)


#if DEFAULT_ASSERT_FUNC 
#include <cassert>

#define winAssert(x)			assert(x)
#define winAssertComment(x, y)	assert(x)

#endif


struct FreeListAllocatorMutex
{
	
	FreeListAllocatorMutex();

	void lock();
	void unlock();
	
	LONG counter;
	HANDLE semaphore;

	~FreeListAllocatorMutex();

};


struct FreeListAllocator
{
	char* baseMemory = 0;
	void *initialBaseMemory = 0;

	FreeListAllocator() = default;
	FreeListAllocator(void* baseMemory, size_t memorySize)
	{
		init(baseMemory, memorySize);
	}

	void init(void* baseMemory, size_t memorySize);

	void *allocate(size_t size);
	void* callocate(size_t size);

	void free(void* mem);

	void* threadSafeAllocate(size_t size);

	void threadSafeFree(void* mem);

	//available memory is the free memory
	//biggest block is how large is the biggest free memory block
	//you can allocate less than the largest biggest free memory because 16 bytes are reserved per block
	void calculateMemoryMetrics(size_t& availableMemory, size_t& biggestBlock, int& freeBlocks);


	//if this is false it will crash if it is out of memory
	//if this is true it will return 0 when there is no more memory
	//I rocommand leaving this to false
	bool returnZeroIfNoMoreMemory = false;

private:

	void* end = 0;

	FreeListAllocatorMutex mu;

	size_t getEnd()
	{
		return (size_t)end;
	}

};


//todo (vlod): implement
#if 0

///set this to 0 if you want to compile on other platforms
#define WINDOWS_DYNAMIC_IMPLEMENTATION 1

struct FreeListAllocatorWinSpecific
{

	FreeListAllocatorWinSpecific() = default;
	FreeListAllocatorWinSpecific(size_t memorySize)
	{
		init(memorySize);
	}

	void init(size_t memorySize);

	void* allocate(size_t size);

	void free(void* mem);

	void* threadSafeAllocate(size_t size);

	void threadSafeFree(void* mem);

	///returns 0 if fails
	bool extendAllocatedMemory(size_t size);

private:
	FreeListAllocator allocator;
	void* endOfReservedSpace;
	void* endOfAllocatedSpace;
	void* beginOfAllocatedSpace;

};

#endif