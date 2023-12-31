#include "CHeapAllocator.hpp"

#include "Windows.h"

#include "Cg.hpp"

#define IN_RANGE(x, a, b) (((x) >= (a)) && ((x) <= (b)))
#define ALIGN(addr, alignment) (((addr) + ((alignment) - 1)) & ~((alignment) - 1))

CHeapAllocator::CHeapAllocator(void)
{
	m_PageEntryArray = nullptr;

	ZeroMemory(&m_FreePages, sizeof(m_FreePages));
}

CHeapAllocator::~CHeapAllocator(void)
{
	CgAssert(m_PageEntryArray == nullptr, L"Page entry array not de-allocated\n");
}

bool CHeapAllocator::Initialize(uint64_t HeapSizeInBytes)
{
	bool status = true;

	if (HeapSizeInBytes % PAGE_SIZE != 0)
	{
		status = false;
		Console::Write(L"Error: Heap size not divisible by the page size\n");
	}

	if (status)
	{
		uint32_t NumPages = static_cast<uint32_t>(HeapSizeInBytes / PAGE_SIZE);
		m_PageEntryArray = reinterpret_cast<PAGE_ENTRY*>(Memory::Allocate(NumPages * sizeof(PAGE_ENTRY), true));

		for (uint32_t i = 0; i < NumPages; i++)
		{
			PAGE_ENTRY* pEntry = &m_PageEntryArray[i];
			pEntry->Offset = i * PAGE_SIZE;

			if (m_FreePages.pHead == nullptr)
			{
				m_FreePages.pHead = pEntry;
				m_FreePages.pTail = pEntry;
			}
			else
			{
				m_FreePages.pTail->pNext = pEntry;
				pEntry->pPrev = m_FreePages.pTail;
				m_FreePages.pTail = pEntry;
			}
		}
	}

	return status;
}

void CHeapAllocator::Uninitialize(void)
{
	if (m_PageEntryArray != nullptr)
	{
		Memory::Release(m_PageEntryArray);
		m_PageEntryArray = nullptr;
	}
}

bool CHeapAllocator::Allocate(uint64_t Size, uint64_t Alignment, uint64_t& rOffset)
{
	bool status = true;

	if (Alignment % KB != 0)
	{
		status = false;
		Console::Write(L"Error: Invalid alignment - not aligned to kilobytes\n");
	}

	if (((Alignment) & (Alignment - 1)) != 0)
	{
		status = false;
		Console::Write(L"Error: Invalid alignment - not a power of 2\n");
	}

	if (status)
	{
		uint32_t NumPages = ALIGN(Size, PAGE_SIZE) / PAGE_SIZE;

		PAGE_ENTRY* pIt = m_FreePages.pHead;
		while (pIt != nullptr)
		{
			if ((Alignment == 0) || ((pIt->Offset & (Alignment - 1)) == 0))
			{
				uint64_t NumFreePages = 1;
				PAGE_ENTRY* pFirstPage = pIt; // The first page of this range

				// Find consecutive free pages
				while (NumFreePages < NumPages)
				{
					if ((pIt->pNext != nullptr) && (pIt->Offset + PAGE_SIZE == pIt->pNext->Offset))
					{
						NumFreePages++;
						pIt = pIt->pNext;
					}
					else
					{
						break;
					}
				}

				if (NumFreePages >= NumPages)
				{
					pIt = pFirstPage;
					break;
				}
			}

			pIt = pIt->pNext;
		}

		// If not null, points to enough pages to store the new allocation
		if (pIt != nullptr)
		{
			ALLOCATION Allocation = {};
			Allocation.Offset = pIt->Offset;
			
			for (uint32_t i = 0; i < NumPages; i++)
			{
				PAGE_ENTRY* pNext = pIt->pNext; //  Store next pointer which will be overwritten

				// Unlink pIt
				if (pIt == m_FreePages.pHead) { m_FreePages.pHead = m_FreePages.pHead->pNext; }
				if (pIt == m_FreePages.pTail) { m_FreePages.pTail = m_FreePages.pTail->pPrev; }

				if (pIt->pPrev != nullptr) { pIt->pPrev->pNext = pIt->pNext; }
				if (pIt->pNext != nullptr) { pIt->pNext->pPrev = pIt->pPrev; }

				pIt->pNext = nullptr;
				pIt->pPrev = nullptr;

				// Insert pIt into the list
				if (Allocation.Entries.pHead == nullptr)
				{
					Allocation.Entries.pHead = pIt;
					Allocation.Entries.pTail = pIt;
				}
				else
				{
					Allocation.Entries.pTail->pNext = pIt;
					pIt->pPrev = Allocation.Entries.pTail;
					Allocation.Entries.pTail = pIt;
				}

				// Prepare for next iteration
				pIt = pNext;
			}

			m_Allocations.push_back(Allocation);

			rOffset = Allocation.Offset;
		}
		else
		{
			status = false;
			Console::Write(L"Error: Could not find space to place the new allocation\n");
		}
	}

	return status;
}

void CHeapAllocator::Free(uint64_t Offset)
{
	std::vector<ALLOCATION>::iterator it = m_Allocations.begin();

	while(it != m_Allocations.end())
	{
		if (it->Offset == Offset)
		{
			break;
		}

		it++;
	}

	if (it != m_Allocations.end())
	{
		ALLOCATION& rAllocation = *it;
		
		if (m_FreePages.pHead == nullptr)
		{
			m_FreePages.pHead = rAllocation.Entries.pHead;
			m_FreePages.pTail = rAllocation.Entries.pTail;
		}
		else
		{
			PAGE_ENTRY* pIt = m_FreePages.pHead;

			while (pIt != nullptr)
			{
				if (rAllocation.Offset < pIt->Offset)
				{
					break;
				}

				pIt = pIt->pNext;
			}

			if (pIt == m_FreePages.pHead)
			{
				rAllocation.Entries.pTail->pNext = m_FreePages.pHead;
				m_FreePages.pHead->pPrev = rAllocation.Entries.pTail;
				m_FreePages.pHead = rAllocation.Entries.pHead;
			}
			else if (pIt == nullptr)
			{
				m_FreePages.pTail->pNext = rAllocation.Entries.pHead;
				rAllocation.Entries.pHead->pPrev = m_FreePages.pTail;
				m_FreePages.pTail = rAllocation.Entries.pTail;
			}
			else
			{
				if (pIt->pPrev != nullptr) { pIt->pPrev->pNext = rAllocation.Entries.pHead; }
				rAllocation.Entries.pHead->pPrev = pIt->pPrev;
				rAllocation.Entries.pTail->pNext = pIt;
				pIt->pPrev = rAllocation.Entries.pTail;
			}
		}

		m_Allocations.erase(it);
	}
	else
	{
		Console::Write(L"Error: Could not find allocation to free\n");
	}
}
