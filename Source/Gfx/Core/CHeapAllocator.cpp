#include "CHeapAllocator.hpp"

#include "Windows.h"

#include "Cg.hpp"

#define IN_RANGE(x, a, b) (((x) >= (a)) && ((x) <= (b)))
#define ALIGN(addr, alignment) (((addr) + ((alignment) - 1)) & ~((alignment) - 1))

const uint64_t CHeapAllocator::PAGE_SIZES[CHeapAllocator::PAGE_SIZE__COUNT] =
{
	256, // PAGE_SIZE__256_BYTE
	512, // PAGE_SIZE__512_BYTE
	1 * KB, // PAGE_SIZE__1_KB
	2 * KB, // PAGE_SIZE__2_KB
	4 * KB, // PAGE_SIZE__4_KB
	8 * KB, // PAGE_SIZE__8_KB 
	16 * KB, // PAGE_SIZE__16_KB
	32 * KB, // PAGE_SIZE__32_KB
	64 * KB, // PAGE_SIZE__64_KB
	128 * KB, // PAGE_SIZE__128_KB
};

CHeapAllocator::CHeapAllocator(void)
{
	ZeroMemory(&m_SortedLists, sizeof(m_SortedLists));
	ZeroMemory(&m_ContiguousList, sizeof(m_ContiguousList));
	ZeroMemory(&m_Chunks, sizeof(m_Chunks));
	ZeroMemory(&m_PageEntries, sizeof(m_PageEntries));
}

CHeapAllocator::~CHeapAllocator(void)
{

}

bool CHeapAllocator::Initialize(uint64_t HeapSizeInBytes)
{
	bool status = true;

	if (HeapSizeInBytes % 2 != 0)
	{
		status = false;
		Console::Write(L"Error: Heap size not a power of 2\n");
	}

	if (status)
	{
		// Initialize the heap with the biggest pages
		PAGE_SIZE PageSize = static_cast<PAGE_SIZE>(PAGE_SIZE__COUNT - 1);

		for (uint64_t Offset = 0; Offset < HeapSizeInBytes; Offset += PAGE_SIZES[PageSize])
		{
			PAGE_ENTRY* pEntry0 = AllocateEntry();
			PAGE_ENTRY* pEntry1 = AllocateEntry();
			if ((pEntry0 != nullptr) && (pEntry1 != nullptr))
			{
				pEntry0->Size = PageSize;
				pEntry0->Offset = Offset;

				pEntry1->Size = PageSize;
				pEntry1->Offset = Offset;

				InsertTail(m_SortedLists[PageSize], pEntry0);
				InsertTail(m_ContiguousList, pEntry1);
			}
			else
			{
				status = false;
				break;
			}
		}
	}

	return status;
}

void CHeapAllocator::Uninitialize(void)
{
	PAGE_CHUNK* pChunk = m_Chunks.pHead;

	while (pChunk != nullptr)
	{
		PAGE_CHUNK* pTemp = pChunk;

		pChunk = pChunk->pNext;

		Memory::Release(pTemp);
	}
}

CHeapAllocator::PAGE_CHUNK* CHeapAllocator::AllocateChunk(void)
{
	uint8_t* pMemory = nullptr;
	PAGE_CHUNK* pChunk = nullptr;
	PAGE_ENTRY* pEntries = nullptr;

	pMemory = reinterpret_cast<uint8_t*>(Memory::Allocate(sizeof(PAGE_CHUNK) + PAGE_ENTRIES_PER_CHUNK * sizeof(PAGE_ENTRY), true));

	if (pMemory != nullptr)
	{
		pChunk = reinterpret_cast<PAGE_CHUNK*>(pMemory);
		pEntries = reinterpret_cast<PAGE_ENTRY*>(pMemory + sizeof(PAGE_CHUNK));

		if (m_Chunks.pHead == nullptr)
		{
			m_Chunks.pHead = pChunk;
			m_Chunks.pTail = pChunk;
		}
		else
		{
			CgAssert(m_Chunks.pTail != nullptr, L"Error: Chunk list tail null\n");

			m_Chunks.pTail->pNext = pChunk;
			m_Chunks.pTail = pChunk;
		}

		for (PAGE_ENTRY* pEntry = pEntries; pEntry != nullptr; pEntry = pEntry->pNext)
		{
			if (m_PageEntries.pHead == nullptr)
			{
				m_PageEntries.pHead = pEntry;
				m_PageEntries.pTail = pEntry;
			}
			else
			{
				m_PageEntries.pTail->pNext = pEntry;
				pEntry->pPrev = m_PageEntries.pTail;
				m_PageEntries.pTail = pEntry;
			}
		}
	}

	return pChunk;
}

CHeapAllocator::PAGE_ENTRY* CHeapAllocator::AllocateEntry(void)
{
	bool status = true;
	PAGE_ENTRY* pEntry = nullptr;

	// Check if there are available entries ahead of time, if there aren't then allocate some
	if (m_PageEntries.pHead == nullptr)
	{
		if (AllocateChunk() == nullptr)
		{
			status = false;
		}
	}

	if (status)
	{
		CgAssert(m_PageEntries.pHead != nullptr, L"Error: No more available entries");
		
		pEntry = m_PageEntries.pHead;
		ZeroMemory(pEntry, sizeof(PAGE_ENTRY));

		m_PageEntries.pHead = m_PageEntries.pHead->pNext;
		m_PageEntries.pHead->pPrev = nullptr;
		if (m_PageEntries.pHead == nullptr)
		{
			m_PageEntries.pTail = nullptr;
		}
	}

	return pEntry;
}

CHeapAllocator::PAGE_ENTRY* CHeapAllocator::PopHead(PAGE_ENTRY_LINKED_LIST& List)
{
	PAGE_ENTRY* pEntry = nullptr;

	if (List.pHead != nullptr)
	{
		pEntry = List.pHead;

		List.pHead = List.pHead->pNext;

		if (List.pHead != nullptr)
		{
			List.pHead->pPrev = nullptr;
		}
	}

	return pEntry;
}

void CHeapAllocator::InsertTail(PAGE_ENTRY_LINKED_LIST& List, PAGE_ENTRY* pEntry)
{
	if (List.pHead == nullptr)
	{
		List.pHead = pEntry;
		List.pTail = pEntry;
	}
	else
	{
		List.pTail->pNext = pEntry;
		pEntry->pPrev = List.pTail;
		List.pTail = pEntry;
	}
}

void CHeapAllocator::RemoveEntry(PAGE_ENTRY_LINKED_LIST& rList, uint64_t Offset, PAGE_SIZE Size)
{
	for (PAGE_ENTRY* pEntry = rList.pHead; pEntry != nullptr; pEntry = pEntry->pNext)
	{
		if ((pEntry->Offset == Offset) && (pEntry->Size == Size))
		{
			if (pEntry->pPrev != nullptr)
			{
				pEntry->pPrev->pNext = pEntry->pNext;
			}

			if (pEntry->pNext != nullptr)
			{
				pEntry->pNext->pPrev = pEntry->pPrev;
			}

			ZeroMemory(pEntry, sizeof(PAGE_ENTRY));
			InsertTail(m_PageEntries, pEntry);

			break;
		}
	}
}

CHeapAllocator::PAGE_SIZE CHeapAllocator::GetPageSize(uint64_t Size)
{
	PAGE_SIZE PageSize = PAGE_SIZE__COUNT;

	if (Size > PAGE_SIZES[PAGE_SIZE__COUNT - 1])
	{
		PageSize = static_cast<PAGE_SIZE>(PAGE_SIZE__COUNT - 1);
	}
	else
	{
		for (uint32_t i = PAGE_SIZE__256_BYTE; i < PAGE_SIZE__COUNT; i++)
		{
			if (Size <= PAGE_SIZES[i])
			{
				PageSize = static_cast<PAGE_SIZE>(i);
				break;
			}
		}
	}

	return PageSize;
}

bool CHeapAllocator::Allocate(uint64_t Size, uint64_t Alignment, uint64_t& rOffset)
{
	bool status = true;

	if (Size <= PAGE_SIZES[PAGE_SIZE__COUNT - 1])
	{
		status = AllocateOnePage(Size, Alignment, rOffset);
	}
	else
	{
		status = AllocateMultiplePages(Size, Alignment, rOffset);
	}

	return status;
}

bool CHeapAllocator::AllocateOnePage(uint64_t Size, uint64_t Alignment, uint64_t& rOffset)
{
	bool status = true;
	PAGE_SIZE PageSize = GetPageSize(Size);
	PAGE_ENTRY* pEntry = nullptr;

	if (m_SortedLists[PageSize].pHead != nullptr)
	{
		pEntry = PopHead(m_SortedLists[PageSize]);
	}
	else
	{
		uint32_t n = PageSize + 1;
		PAGE_ENTRY* pCandidatePage = nullptr;

		// Start search for the next page size this allocation be placed inside
		while (n < PAGE_SIZE__COUNT)
		{
			for (PAGE_ENTRY* pPage = m_SortedLists[n].pHead; (pCandidatePage == nullptr) && (pPage != nullptr); pPage = pPage->pNext)
			{
				if (ALIGN(pPage->Offset, Alignment) <= pPage->Offset + PAGE_SIZES[n])
				{
					pCandidatePage = pPage;
					break;
				}
			}

			if (pCandidatePage != nullptr) { break; }
			else                           { n++;   }
		}

		// If a valid page is found, break it down to get the desired page size
		if (pCandidatePage != nullptr)
		{
			PAGE_ENTRY_LINKED_LIST NewPages[PAGE_SIZE__COUNT] = {};

			// Remove this candidage page from the contiguous array before starting
			RemoveEntry(m_ContiguousList, pCandidatePage->Offset, pCandidatePage->Size);

			while (status && (n >= PageSize))
			{
				uint64_t SubPageOffset = pCandidatePage->Offset;

				// Unlink the candidate
				if (pCandidatePage->pPrev != nullptr)
				{
					pCandidatePage->pPrev->pNext = pCandidatePage->pNext;
				}

				if (pCandidatePage->pNext != nullptr)
				{
					pCandidatePage->pNext->pPrev = pCandidatePage->pPrev;
				}

				pCandidatePage->pPrev = nullptr;
				pCandidatePage->pNext = nullptr;

				ZeroMemory(pCandidatePage, sizeof(PAGE_ENTRY));
				InsertTail(m_PageEntries, pCandidatePage);
				pCandidatePage = nullptr;

				// Break apart candidate page into subpages
				uint64_t NumSubEntries = PAGE_SIZES[n] / PAGE_SIZES[n-1];

				PAGE_ENTRY_LINKED_LIST& rSubPages = NewPages[n-1];

				for (uint32_t i = 0; i < NumSubEntries; i++)
				{
					PAGE_ENTRY* pSubPage = AllocateEntry();

					if (pSubPage == nullptr)
					{
						status = false;
						break;
					}

					pSubPage->Size = static_cast<PAGE_SIZE>(n-1);
					pSubPage->Offset = SubPageOffset + i * PAGE_SIZES[n-1];

					InsertTail(rSubPages, pSubPage);
				}

				// Find the next candidate page
				for (PAGE_ENTRY* pPage = rSubPages.pHead; pPage != nullptr; pPage = pPage->pNext)
				{
					if (ALIGN(pPage->Offset, Alignment) <= pPage->Offset + PAGE_SIZES[n-1])
					{
						pCandidatePage = pPage;
						break;
					}
				}

				if (pCandidatePage == nullptr)
				{
					status = false;
					Console::Write(L"Error: Could not find candidate page\n");
					break;
				}
			
				// Move onto next iteration
				n--;
			}

			// TODO: INSERT ALL NEW PAGES INTO CURRENT FREE PAGES

			if (pCandidatePage != nullptr)
			{
				pEntry = pCandidatePage;
			}
			else
			{
				status = false;
				Console::Write(L"Error: Failed to break down candidate page\n");
			}
		}
	}

	if (status && (pEntry != nullptr))
	{
		ALLOCATION Allocation = {};
		Allocation.Offset = pEntry->Offset;
	}

	return status;
}

bool CHeapAllocator::AllocateMultiplePages(uint64_t Size, uint64_t Alignment, uint64_t& rOffset)
{
	bool status = true;

	return status;
}
