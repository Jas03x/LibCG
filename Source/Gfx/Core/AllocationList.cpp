#include "AllocationList.hpp"

#include "Windows.h"

#include "Cg.hpp"

AllocationList::AllocationList(void)
{
	ZeroMemory(&m_AllocatedEntries, sizeof(m_AllocatedEntries));
	ZeroMemory(&m_FreeEntries, sizeof(m_FreeEntries));
	ZeroMemory(&m_EntryArray, sizeof(m_EntryArray));

	m_FreeEntries.pHead = &m_EntryArray[0];
	m_FreeEntries.pTail = &m_EntryArray[0];

	for (uint32_t i = 1; i < MAX_ENTRIES; i++)
	{
		m_FreeEntries.pTail->pNext = &m_EntryArray[i];
		m_FreeEntries.pTail = &m_EntryArray[i];
	}
}

AllocationList::~AllocationList(void)
{

}

bool AllocationList::Allocate(ID3D12Resource* pAllocation)
{
	bool status = true;
	Entry* pEntry = nullptr;

	if (pAllocation == nullptr)
	{
		status = false;
		Console::Write(L"Error: Null allocation pointer\n");
	}

	if (status)
	{
		if (m_FreeEntries.pHead != nullptr)
		{
			pEntry = m_FreeEntries.pHead; 
			m_FreeEntries.pHead = m_AllocatedEntries.pHead->pNext;
			ZeroMemory(pEntry, sizeof(Entry));
		}
		else
		{
			status = false;
			Console::Write(L"Error: Linked list out of memory\n");
		}
	}

	if (status)
	{
		pEntry->pAllocation = pAllocation;
		m_AllocatedEntries.pTail->pNext = pEntry;
		m_AllocatedEntries.pTail = pEntry;
	}

	return status;
}

bool AllocationList::Free(ID3D12Resource* pAllocation)
{
	bool status = true;
	Entry* pEntry = nullptr;
	Entry* pPrevEntry = nullptr;

	for (Entry* it = m_AllocatedEntries.pHead; it != nullptr; it = it->pNext)
	{
		if (it->pAllocation == pAllocation)
		{
			pEntry = it;
			break;
		}
		
		pPrevEntry = it;
	}

	if (pEntry == nullptr)
	{
		status = false;
		Console::Write(L"Error: Allocation not in heap\n");
	}

	if (status)
	{
		if (pPrevEntry != nullptr)
		{
			pPrevEntry->pNext = pEntry->pNext;
		}
		else
		{
			m_AllocatedEntries.pHead = pEntry->pNext;
		}

		ZeroMemory(pEntry, sizeof(Entry));
		pEntry->pNext = m_FreeEntries.pHead;
		m_FreeEntries.pHead = pEntry;
	}

	return status;
}
