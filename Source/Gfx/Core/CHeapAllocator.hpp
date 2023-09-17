#ifndef CG_HEAP_ALLOCATOR_HPP
#define CG_HEAP_ALLOCATOR_HPP

#include "CgDef.hpp"

#include <vector>

class CHeapAllocator
{
private:
	enum PAGE_SIZE : uint32_t
	{
		PAGE_SIZE = 1 * KB
	};

	struct PAGE_ENTRY
	{
		uint64_t    Offset;
		PAGE_ENTRY* pNext;
		PAGE_ENTRY* pPrev;
	};

	struct PAGE_ENTRY_LINKED_LIST
	{
		PAGE_ENTRY* pHead;
		PAGE_ENTRY* pTail;
	};

	struct ALLOCATION
	{
		uint64_t               Offset;
		PAGE_ENTRY_LINKED_LIST Entries;
	};

	std::vector<ALLOCATION> m_Allocations;

	PAGE_ENTRY_LINKED_LIST  m_FreePages;

	PAGE_ENTRY*             m_PageEntryArray;

public:
	CHeapAllocator(void);
	~CHeapAllocator(void);

	bool        Initialize(uint64_t HeapSizeInBytes);
	void        Uninitialize(void);

	bool        Allocate(uint64_t Size, uint64_t Alignment, uint64_t& rOffset);
	void        Free(uint64_t Offset);
};

#endif // CG_HEAP_ALLOCATOR_HPP