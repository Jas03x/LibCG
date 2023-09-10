#ifndef CG_HEAP_ALLOCATOR_HPP
#define CG_HEAP_ALLOCATOR_HPP

#include "CgDef.hpp"

#include <vector>

class CHeapAllocator
{
private:
	enum PAGE_SIZE : uint32_t
	{
		PAGE_SIZE__256_BYTE = 0,
		PAGE_SIZE__512_BYTE = 1,
		PAGE_SIZE__1_KB = 2,
		PAGE_SIZE__2_KB = 3,
		PAGE_SIZE__4_KB = 4,
		PAGE_SIZE__8_KB = 5,
		PAGE_SIZE__16_KB = 6,
		PAGE_SIZE__32_KB = 7,
		PAGE_SIZE__64_KB = 8,
		PAGE_SIZE__128_KB = 9,
		PAGE_SIZE__COUNT = 10
	};

	static const uint64_t PAGE_SIZES[PAGE_SIZE__COUNT];

	struct PAGE_ENTRY
	{
		PAGE_SIZE   Size;
		uint64_t    Offset;
		PAGE_ENTRY* pNext;
		PAGE_ENTRY* pPrev;
	};

	struct PAGE_CHUNK
	{
		PAGE_ENTRY* pEntries;
		PAGE_CHUNK* pNext;
	};

	// A chunk should take about 4KB
	enum { PAGE_ENTRIES_PER_CHUNK = (4 * KB - sizeof(PAGE_CHUNK)) / sizeof(PAGE_ENTRY) };

	struct PAGE_ENTRY_LINKED_LIST
	{
		PAGE_ENTRY* pHead;
		PAGE_ENTRY* pTail;
	};

	struct PAGE_CHUNK_LINKED_LIST
	{
		PAGE_CHUNK* pHead;
		PAGE_CHUNK* pTail;
	};

	struct ALLOCATION
	{
		uint64_t               Offset;
		PAGE_ENTRY_LINKED_LIST Entries;
	};

	std::vector<ALLOCATION> m_Allocations;

	PAGE_ENTRY_LINKED_LIST  m_SortedLists[PAGE_SIZE__COUNT]; // All pages available to be used (per block size)
	PAGE_ENTRY_LINKED_LIST  m_ContiguousList; // All available pages stored contiguously

	PAGE_CHUNK_LINKED_LIST  m_Chunks;      // All page chunks
	PAGE_ENTRY_LINKED_LIST  m_PageEntries; // All available page entries

public:
	CHeapAllocator(void);
	~CHeapAllocator(void);

	bool        Initialize(uint64_t HeapSizeInBytes);
	void        Uninitialize(void);

	bool        Allocate(uint64_t Size, uint64_t Alignment, uint64_t& rOffset);

private:
	PAGE_CHUNK* AllocateChunk(void);
	PAGE_ENTRY* AllocateEntry(void);

	bool        AllocateOnePage(uint64_t Size, uint64_t Alignment, uint64_t& rOffset);
	bool        AllocateMultiplePages(uint64_t Size, uint64_t Alignment, uint64_t& rOffset);

	PAGE_ENTRY* PopHead(PAGE_ENTRY_LINKED_LIST& List);
	void        InsertTail(PAGE_ENTRY_LINKED_LIST& List, PAGE_ENTRY* pEntry);
	void        RemoveEntry(PAGE_ENTRY_LINKED_LIST& rList, uint64_t Offset, PAGE_SIZE Size);

	PAGE_SIZE   GetPageSize(uint64_t Size);
};

#endif // CG_HEAP_ALLOCATOR_HPP