#include "CHeapAllocator.hpp"

#include "Windows.h"

#include "Cg.hpp"

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
	ZeroMemory(&m_FreePages, sizeof(m_FreePages));
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
		PAGE_SIZE page_size = static_cast<PAGE_SIZE>(PAGE_SIZE__COUNT - 1);

		for (uint64_t offset = 0; offset < HeapSizeInBytes; offset += PAGE_SIZES[page_size])
		{
			PAGE_ENTRY* pEntry = AllocateEntry();
			if (pEntry != nullptr)
			{
				pEntry->Offset = offset;
				InsertTail(m_FreePages[page_size], pEntry);
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

		m_PageEntries.pHead = m_PageEntries.pHead->pNext;
		m_PageEntries.pHead->pPrev = nullptr;
		if (m_PageEntries.pHead == nullptr)
		{
			m_PageEntries.pTail = nullptr;
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

void CHeapAllocator::InsertSequence(PAGE_ENTRY_LINKED_LIST& Dst, PAGE_ENTRY_LINKED_LIST& Src)
{
	if (Dst.pHead == nullptr)
	{
		Dst.pHead = Src.pHead;
		Dst.pTail = Src.pHead;
	}
	else
	{
		for (PAGE_ENTRY* pEntry = Dst.pHead; pEntry != nullptr; pEntry = pEntry->pNext)
		{
			if (Src.pHead->Offset < pEntry->Offset)
			{
				if (pEntry == Dst.pHead)
				{
					pEntry->pPrev = Src.pTail;
					Src.pTail->pNext = pEntry;
					Dst.pHead = pEntry;
				}
				else
				{

				}

				break;
			}
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

bool CHeapAllocator::Allocate(uint64_t Size, uint64_t& Offset)
{
	bool status = true;

	if (Size <= PAGE_SIZES[PAGE_SIZE__COUNT - 1])
	{
		status = AllocateOnePage(Size, Offset);
	}
	else
	{
		status = AllocateMultiplePages(Size, Offset);
	}

	return status;
}

bool CHeapAllocator::AllocateOnePage(uint64_t Size, uint64_t& Offset)
{
	bool status = true;
	PAGE_SIZE PageSize = GetPageSize(Size);
	PAGE_ENTRY* pEntry = nullptr;

	// Check if pages will need to be split up
	if (m_FreePages[PageSize].pHead == nullptr)
	{
		// Break apart larger pages
		uint32_t n = PageSize + 1;
		while (n < PAGE_SIZE__COUNT)
		{
			if (m_FreePages[n].pHead != nullptr)
			{
				break;
			}

			n++;
		}

		if (n < PAGE_SIZE__COUNT)
		{
			while (n > PageSize)
			{
				PAGE_ENTRY* pLargePage = m_FreePages[n].pHead;
				if (pLargePage == nullptr)
				{
					status = false;
					break;
				}

				uint64_t Offset = pLargePage->Offset;
				uint32_t NumSubPages = PAGE_SIZES[n] / PAGE_SIZES[n-1];

				m_FreePages[n].pHead = pLargePage->pNext;
				m_FreePages[n].pHead->pPrev = nullptr;
				pLargePage->pNext = nullptr;

				InsertTail(m_PageEntries, pLargePage);

				PAGE_ENTRY_LINKED_LIST SmallPages = {};

				for (uint32_t i = 0; i < NumSubPages; i++)
				{
					PAGE_ENTRY* small_page = AllocateEntry();
					if (small_page != nullptr)
					{
						small_page->Offset = Offset + i * PAGE_SIZES[n-1];
						InsertTail(SmallPages, small_page);
					}
					else
					{
						status = false;
						break;
					}
				}

				if (status)
				{
					InsertSequence(m_FreePages[n], SmallPages);
				}
				else
				{
					break;
				}
			}
		}
		else
		{
			status = false;
		}
	}

	if (status)
	{
		if (m_FreePages[PageSize].pHead != nullptr)
		{
			pEntry = m_FreePages[PageSize].pHead;
			m_FreePages[PageSize].pHead = pEntry->pNext;
			m_FreePages[PageSize].pHead->pPrev = nullptr;
		}
		else
		{
			status = false;
		}
	}

	if (status)
	{
		ALLOCATION Allocation = {};
		Allocation.Offset = pEntry->Offset;
	}

	return status;
}

bool CHeapAllocator::AllocateMultiplePages(uint64_t Size, uint64_t& Offset)
{
	bool status = true;

	return status;
}
