#include "Cg.hpp"
#include "CMemory.hpp"

#include <assert.h>

#include <windows.h>

HANDLE   CMemory::m_hHeap = NULL;
uint64_t CMemory::m_nBytesAllocated = 0;

PVOID Memory::Allocate(size_t nBytes, bool bClear)
{
	return CMemory::Allocate(nBytes, bClear);
}

bool Memory::Release(void* pMemory)
{
	return CMemory::Release(pMemory);
}

bool CMemory::Initialize(void)
{
	bool status = true;

	m_nBytesAllocated = 0;

	m_hHeap = GetProcessHeap();

	if (m_hHeap == NULL)
	{
		status = false;
	}

	return status;
}

bool CMemory::Uninitialize(void)
{
	bool status = true;

	m_hHeap = NULL;

	if (m_nBytesAllocated != 0)
	{
		const WCHAR ErrorMsg[] = __FUNCTION__ L": ERROR - Memory leak detected\n";

		HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

		if (hStdOut != NULL)
		{
			WriteConsole(hStdOut, ErrorMsg, _countof(ErrorMsg), NULL, NULL);
			status = false;
		}
	}

	return status;
}

PVOID CMemory::Allocate(size_t nBytes, bool bClear)
{
	PVOID pAlloc = NULL;
	DWORD Flags = 0;

	if (bClear)
	{
		Flags |= HEAP_ZERO_MEMORY;
	}

	pAlloc = HeapAlloc(m_hHeap, Flags, nBytes);

	if (pAlloc != NULL)
	{
		m_nBytesAllocated += nBytes;
	}

	return pAlloc;
}

bool CMemory::Release(void* pMemory)
{
	bool status = true;

	if (pMemory != NULL)
	{
		size_t nBytes = HeapSize(m_hHeap, 0, pMemory);

		if (nBytes != static_cast<size_t>(-1))
		{
			m_nBytesAllocated -= nBytes;
		}
	}

	status = HeapFree(m_hHeap, 0, pMemory);

	return status;
}
