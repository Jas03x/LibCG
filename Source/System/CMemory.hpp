#ifndef CG_CMEMORY_HPP
#define CG_CMEMORY_HPP

#include "CgDef.hpp"

class CMemory
{
protected:
	static HANDLE   m_hHeap;
	static uint64_t m_nBytesAllocated;

public:
	static bool Initialize(void);
	static bool Uninitialize(void);

	static void* Allocate(size_t nBytes, bool bClear);
	static bool  Release(void* pMemory);
};

#endif // CG_CMEMORY_HPP