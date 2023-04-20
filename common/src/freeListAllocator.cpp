//////////////////////////////////////////////////
//freeListAllocator.h				beta 0.3
//Copyright(c) 2020 Luta Vlad
//https://github.com/meemknight/freeListAllocator
//////////////////////////////////////////////////


#include "freeListAllocator.h"
#include <cstdint>
#include <algorithm>

#include <intrin.h>
#pragma intrinsic (_InterlockedIncrement)
#pragma intrinsic (_InterlockedDecrement)

#if LINK_TO_GLOBAL_ALLOCATOR == 1


#if WINDOWS_DYNAMIC_IMPLEMENTATION == 1

FreeListAllocatorWinSpecific allocator(GB(15));

#else

char memBlock[HEAP_SIZE] = {};
FreeListAllocator allocator(memBlock, sizeof(memBlock));

#endif


//todo (vlod): Add c++17 overloads
//todo add own assert

void* operator new  (std::size_t count)
{
	auto a = allocator.allocate(count);

	//std::cout << "Allocated " << count << " at " << a << '\n';

	return a;
}

void* operator new[](std::size_t count)
{
	auto a = allocator.allocate(count);

	//std::cout << "Allocated " << count << " at " << a << '\n';

	return a;
}

void operator delete  (void* ptr)
{
	//std::cout << "Deallocated at: " << ptr << "\n";

	allocator.free(ptr);
}

void operator delete[](void* ptr)
{
	//std::cout << "Deallocated at: " << ptr << "\n";

	allocator.free(ptr);
}

#endif // LINK_TO_GLOBAL_ALLOCATOR


const uint64_t GUARD_VALUE = 0xff'ff'ff'ff'ff'ff'ff'ff;

struct FreeBlock
{
	union
	{
		char* next;
		std::uint64_t dummy_;
	};
	std::uint64_t size;

};

struct AllocatedBlock
{
	std::uint64_t size;
	std::uint64_t guard;
};


void FreeListAllocator::init(void* baseMemory, size_t memorySize)
{
	end = (void*)((size_t)baseMemory + memorySize);

	static_assert(sizeof(FreeBlock) == sizeof(AllocatedBlock), "");

	winAssertComment(memorySize > 100, "memory size must be greater than 100 bytes");

	this->baseMemory = (char*)baseMemory;
	this->initialBaseMemory = baseMemory;

	//move base memory to a 8 byte aligned

	size_t pos = (int)this->baseMemory;

	if (pos % 8 != 0)
	{
		size_t newMem = ((size_t)this->baseMemory | 0b111);
		memorySize -= (newMem)-(size_t)this->baseMemory;
		this->baseMemory = (char *)newMem;
	}

	((FreeBlock*)this->baseMemory)->next = nullptr;
	((FreeBlock*)this->baseMemory)->size = memorySize - sizeof(FreeBlock);


}

void* FreeListAllocator::allocate(size_t size)
{

	winAssert(baseMemory, "Allocator not initialized"); //err allocator not initialized



	FreeBlock* last = nullptr;
	FreeBlock* current = (FreeBlock*)baseMemory;

	int aligned8Size = size;
	if (aligned8Size % 8 != 0)
	{
		aligned8Size += (8 - (aligned8Size % 8));
	}

	while (true)
	{


		if (aligned8Size <= ((FreeBlock*)current)->size) // this is a suitable block
		{
			//add block here


			FreeBlock* next = (FreeBlock*)current->next;

			if (next == nullptr || next >= this->end) //this is the last block
			{
				if (last == nullptr || last >= this->end) //this is also the first block so move the base pointer
				{
					void* toReturn = (char*)current + sizeof(AllocatedBlock);
					((AllocatedBlock*)current)->size = aligned8Size;	//size of the new allocated block
					((AllocatedBlock*)current)->guard = GUARD_VALUE;

					FreeBlock* nextFreeBlock = (FreeBlock*)((char*)toReturn + aligned8Size);
					//next free block is the base memory now

					baseMemory = (char*)nextFreeBlock;
					((FreeBlock*)baseMemory)->next = nullptr; //this is the last block

					size_t size = this->getEnd() - (size_t)nextFreeBlock - (size_t)sizeof(FreeBlock); //set the size of the new last block
					((FreeBlock*)baseMemory)->size = size;

					return toReturn;
				}
				else //this is not the first block so change the last block pointer
				{
					void* toReturn = (char*)current + sizeof(AllocatedBlock);
					((AllocatedBlock*)current)->size = aligned8Size;	//size of the new allcoated block
					((AllocatedBlock*)current)->guard = GUARD_VALUE;

					FreeBlock* nextFreeBlock = (FreeBlock*)((char*)toReturn + aligned8Size);

					last->next = (char*)nextFreeBlock; //last is relinked
					nextFreeBlock->next = nullptr; //this is the last block

					size_t size = this->getEnd() - (size_t)nextFreeBlock - (size_t)sizeof(FreeBlock); //set the size of the new last block
					nextFreeBlock->size = size;

					return toReturn;
				}

			}
			else //this is not the last free block 
			{

				if (last == nullptr || last >= this->end) // this is the first free block but not the last 
				{
					size_t currentSize = ((FreeBlock*)current)->size;

					void* toReturn = (char*)current + sizeof(AllocatedBlock);

					if (currentSize - aligned8Size < 24)
					{
						//too small block remaining

						if (currentSize - aligned8Size < 0 || (currentSize - aligned8Size) % 8 != 0)
						{
							//heap corrupted
							winAssert(0, "heap corupted");
						}

						aligned8Size += (currentSize - aligned8Size);

						((AllocatedBlock*)current)->size = aligned8Size;	//size of the new allocated block
						((AllocatedBlock*)current)->guard = GUARD_VALUE;

						FreeBlock* nextFreeBlock = next;
						//next free block is the next block

						baseMemory = (char*)nextFreeBlock;

						return toReturn;

					}
					else
					{
						//add a new block
						FreeBlock* newCreatedBlock = (FreeBlock*)((char*)toReturn + aligned8Size);
						newCreatedBlock->size = currentSize - aligned8Size - sizeof(AllocatedBlock);
						newCreatedBlock->next = (char*)next;

						((AllocatedBlock*)current)->size = aligned8Size;	//size of the new allocated block
						((AllocatedBlock*)current)->guard = GUARD_VALUE;
						baseMemory = (char*)newCreatedBlock;

						return toReturn;
					}



				}
				else // this is not the first free block and not the last 
				{	//todo fix here

					void* toReturn = (char*)current + sizeof(AllocatedBlock);
					size_t currentSize = ((FreeBlock*)current)->size;

					if (currentSize - aligned8Size < 24)
					{

						//too small block remaining
						if (currentSize - aligned8Size < 0 || (currentSize - aligned8Size) % 8 != 0)
						{
							//heap corrupted
							winAssert(0, "heap corupted");
						}

						aligned8Size += (currentSize - aligned8Size);

						((AllocatedBlock*)current)->size = aligned8Size;	//size of the new allcoated block
						((AllocatedBlock*)current)->guard = GUARD_VALUE;

						//FreeBlock* nextFreeBlock = (FreeBlock*)((char*)toReturn + aligned8Size);

						last->next = (char*)next; //last is relinked
						//nextFreeBlock->next = (char*)next; //this is not the last block

						//size_t size = this->getEnd() - (size_t)nextFreeBlock - (size_t)sizeof(FreeBlock); //set the size of the new last block
						//nextFreeBlock->size = size;

						return toReturn;

					}
					else
					{
						//add a new block
						FreeBlock* newCreatedBlock = (FreeBlock*)((char*)toReturn + aligned8Size);
						newCreatedBlock->size = currentSize - aligned8Size - sizeof(AllocatedBlock);
						newCreatedBlock->next = (char*)next;

						((AllocatedBlock*)current)->size = aligned8Size;	//size of the new allcoated block
						((AllocatedBlock*)current)->guard = GUARD_VALUE;

						//FreeBlock* nextFreeBlock = (FreeBlock*)((char*)toReturn + aligned8Size);

						last->next = (char*)newCreatedBlock; //last is relinked
						//nextFreeBlock->next = (char*)next; //this is not the last block

						//size_t size = this->getEnd() - (size_t)nextFreeBlock - (size_t)sizeof(FreeBlock); //set the size of the new last block
						//nextFreeBlock->size = size;

						return toReturn;

					}


				}
			}

		}
		else
		{
			if (current->next == nullptr || current->next >= this->end)
			{
				//that was the last block, no size
				//std::cout << "no more memory\n";

				if (returnZeroIfNoMoreMemory)
				{
					return 0;
				}
				else
				{
					winAssertComment(0, "Allocator out of memory");
				}

			}
			else
			{
				last = current;
				current = (FreeBlock*)current->next;

			}

		}


	}

	winAssert(0);
	return nullptr;
}

void FreeListAllocator::free(void* mem)
{
	if (mem == nullptr) { return; }

	char* memoryBegin = (char*)mem;

	//mem is the beginning of the valid memory, \
	the header is then 16 bytes lower

	void* headerBegin = memoryBegin - sizeof(AllocatedBlock);

	AllocatedBlock* allocatedBLockHeader = (AllocatedBlock*)headerBegin;

#pragma region check validity

	winAssertComment(allocatedBLockHeader->guard == GUARD_VALUE, "invalid free or double free"); //invalid free or double free

#pragma endregion

	size_t sizeOfTheFreedBlock = allocatedBLockHeader->size;

	if (headerBegin < this->baseMemory)
	{
		//the freed memory is before the base memory so change the base memory
		//this is the new base memory

		if ((size_t)headerBegin + sizeof(AllocatedBlock) + sizeOfTheFreedBlock == (size_t)this->baseMemory)
		{
			//this merges with the current first free block so merge them

			FreeBlock* firstFreeBlock = (FreeBlock*)allocatedBLockHeader;
			firstFreeBlock->next = ((FreeBlock*)this->baseMemory)->next;
			firstFreeBlock->size = sizeOfTheFreedBlock + sizeof(FreeBlock) + ((FreeBlock*)this->baseMemory)->size;

			this->baseMemory = (char*)firstFreeBlock;

		}
		else if ((size_t)headerBegin + sizeof(AllocatedBlock) + sizeOfTheFreedBlock > (size_t)this->baseMemory)
		{
			//heap corupted
			winAssert(0, "heap corupted");
		}
		else
		{	//this doesn't merge with the next free block so just link them
			FreeBlock* firstFreeBlock = (FreeBlock*)allocatedBLockHeader;

			firstFreeBlock->next = this->baseMemory;
			firstFreeBlock->size = sizeOfTheFreedBlock;

			this->baseMemory = (char*)firstFreeBlock;
		}


	}
	else
	{
		//the freed block is somewhere in the middle
		//first search for the blocks before and after it

		FreeBlock* current = (FreeBlock*)baseMemory;
		FreeBlock* next = (FreeBlock*)current->next;

		while (true)
		{

			if ((current < headerBegin && headerBegin < next)
				||
				(current < headerBegin && (next == nullptr || next >= this->end))
				)
			{
				//the block is between 2 blocks
				FreeBlock* theBlockBefore = current;
				FreeBlock* theBlockAfter = (FreeBlock*)current->next;

				//merge with the block before

				FreeBlock* newCurent = nullptr;
				//check if merged
				if ((size_t)theBlockBefore + sizeof(FreeBlock) + theBlockBefore->size == (size_t)headerBegin)
				{
					//merge
					theBlockBefore->size += sizeof(FreeBlock) + sizeOfTheFreedBlock;

					newCurent = theBlockBefore;
				}
				else if ((size_t)theBlockBefore + sizeof(FreeBlock) + theBlockBefore->size > (size_t)headerBegin)
				{
					//error heap corupted
					winAssert(0, "heap corupted");
				}
				else
				{
					//just link, no merge
					theBlockBefore->next = (char*)headerBegin;
					FreeBlock* thisBlock = (FreeBlock*)headerBegin;
					thisBlock->next = nullptr;
					thisBlock->size = sizeOfTheFreedBlock;

					newCurent = thisBlock;
				}

				//merge / link with next block
				if (theBlockAfter != nullptr && theBlockAfter < this->end)
				{

					if ((size_t)newCurent + sizeof(FreeBlock) + newCurent->size == (size_t)theBlockAfter)
					{
						//merge
						newCurent->size += sizeof(FreeBlock) + theBlockAfter->size;
						newCurent->next = theBlockAfter->next;

					}
					else if ((size_t)newCurent + sizeof(FreeBlock) + newCurent->size > (size_t)theBlockAfter)
					{
						//err
						winAssert(0, "heap corupted");
					}
					else
					{
						//just link
						newCurent->next = (char*)theBlockAfter;

					}

				}


				break;
			}

			current = (FreeBlock*)current->next;
			next = (FreeBlock*)current->next;

			if (current == nullptr || current >= this->end)
			{
				//heap corupted or freed an invalid value
				winAssert(0, "heap corupted or an invalid free");

			}
		}
	}

}


void* FreeListAllocator::threadSafeAllocate(size_t size)
{
	mu.lock();

	auto a = this->allocate(size);

	mu.unlock();

	return a;
}

void FreeListAllocator::threadSafeFree(void* mem)
{
	if (mem == nullptr) { return; }

	mu.lock();

	this->free(mem);

	mu.unlock();
}

void FreeListAllocator::calculateMemoryMetrics(size_t& availableMemory, size_t& biggestBlock, int& freeBlocks)
{

	availableMemory = 0;
	biggestBlock = 0;
	freeBlocks = 0;

	if (!baseMemory) 
	{		
		return;
	}


	FreeBlock* last = nullptr;
	FreeBlock* current = (FreeBlock*)baseMemory;

	while (true)
	{
		freeBlocks++;
		availableMemory += current->size;
		biggestBlock = std::max(biggestBlock, current->size);

		if (current->next == nullptr || current->next >= this->end)
		{
			//that was the last block

			break;

		}
		else
		{
			last = current;
			current = (FreeBlock*)current->next;
		}


	}

	

}



#if WINDOWS_DYNAMIC_IMPLEMENTATION == 1

#include <Windows.h>


void FreeListAllocatorWinSpecific::init(size_t memorySize)
{
	auto base = VirtualAlloc(nullptr, memorySize, MEM_RESERVE, PAGE_READWRITE);

	beginOfAllocatedSpace = base;
	endOfReservedSpace = (char*)base + memorySize;
	endOfAllocatedSpace = base;

	assert(extendAllocatedMemory(KB(1)));

	allocator.returnZeroIfNoMoreMemory = true;
	allocator.init(base, memorySize);

}

inline void* FreeListAllocatorWinSpecific::allocate(size_t size)
{
	void* rez = 0;

	while (true)
	{
		rez = allocator.allocate(size);

		if (rez == nullptr)
		{
			assert(this->extendAllocatedMemory(size));
		}
		else
		{
			break;
		}
	}

	return rez;

}

inline void FreeListAllocatorWinSpecific::free(void* mem)
{
	allocator.free(mem);
}

inline void* FreeListAllocatorWinSpecific::threadSafeAllocate(size_t size)
{
	void* rez = 0;

	while (true)
	{
		rez = allocator.threadSafeAllocate(size);

		if (rez == nullptr)
		{
			assert(this->extendAllocatedMemory(size));
		}
		else
		{
			break;
		}
	}

	return rez;

}

inline void FreeListAllocatorWinSpecific::threadSafeFree(void* mem)
{
	allocator.threadSafeFree(mem);
}

bool FreeListAllocatorWinSpecific::extendAllocatedMemory(size_t size)
{
	if ((char*)endOfAllocatedSpace + size > endOfReservedSpace)
	{
		return false;
	}

	size_t sizeToAllcoate = ((char*)endOfAllocatedSpace - (char*)beginOfAllocatedSpace) + size;

	auto rez = VirtualAlloc(beginOfAllocatedSpace, sizeToAllcoate,
		MEM_COMMIT, PAGE_READWRITE);


	if (!rez)
	{
		return false;
	}

	endOfAllocatedSpace = ((char*)endOfAllocatedSpace) + size;


	return true;
}

#endif

//https://preshing.com/20120226/roll-your-own-lightweight-mutex/
FreeListAllocatorMutex::FreeListAllocatorMutex()
{
	counter = 0;
	semaphore = CreateSemaphore(NULL, 0, 1, NULL);
}

void FreeListAllocatorMutex::lock()
{
	
	if (_InterlockedIncrement(&counter) > 1) // x86/64 guarantees acquire semantics
	{
		WaitForSingleObject(semaphore, INFINITE);
	}

}

void FreeListAllocatorMutex::unlock()
{
	if (_InterlockedDecrement(&counter) > 0) // x86/64 guarantees release semantics
	{
		ReleaseSemaphore(semaphore, 1, NULL);
	}
}

FreeListAllocatorMutex::~FreeListAllocatorMutex()
{
	CloseHandle(semaphore);
	counter = 0;
	semaphore = 0;
}
