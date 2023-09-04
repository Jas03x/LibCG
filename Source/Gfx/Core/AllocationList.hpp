#ifndef CG_ALLOCATION_LIST_HPP
#define CG_ALLOCATION_LIST_HPP

struct ID3D12Resource;

class AllocationList
{
private:
	struct Entry
	{
		ID3D12Resource* pAllocation;
		Entry* pNext;
	};

	struct List
	{
		Entry* pHead;
		Entry* pTail;
	};

	// TODO: Make this dynamic
	enum { MAX_ENTRIES = 16 };

	List m_AllocatedEntries;
	List m_FreeEntries;

	Entry m_EntryArray[MAX_ENTRIES];

public:
	AllocationList(void);
	~AllocationList(void);

	bool Allocate(ID3D12Resource* pAllocation);
	bool Free(ID3D12Resource* pAllocation);
};

#endif // CG_ALLOCATION_LIST_HPP