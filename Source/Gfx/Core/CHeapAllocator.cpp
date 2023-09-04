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
	ZeroMemory(&m_UsedPages, sizeof(m_UsedPages));
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
			if (!InsertEntry(page_size, offset, false))
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

bool CHeapAllocator::InsertEntry(PAGE_SIZE Size, uint64_t Offset, bool Allocated)
{
	bool status = true;
	PAGE_ENTRY* pEntry = AllocateEntry();

	if (pEntry != nullptr)
	{
		pEntry->Offset = Offset;

		PAGE_ENTRY_LINKED_LIST& List = Allocated ? m_UsedPages[Size] : m_FreePages[Size];
		if (List.pHead == nullptr)
		{
			List.pHead = pEntry;
			List.pTail = pEntry;
		}
		else
		{
			CgAssert(List.pTail != nullptr, L"Error: Page entry list tail is null\n");

			List.pTail->pNext = pEntry;
			pEntry->pPrev = List.pTail;
			List.pTail = pEntry;
		}
	}
	else
	{
		status = false;
	}

	return status;
}
