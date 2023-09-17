#ifndef CG_CHEAP_HPP
#define CG_CHEAP_HPP

#include "CgDef.hpp"

#include "CHeapAllocator.hpp"

enum   D3D12_RESOURCE_STATES;

struct D3D12_RESOURCE_DESC;

struct ID3D12Heap;
struct ID3D12Resource;
struct ID3D12Device4;

class  CAllocation;

class CHeap
{
private:
	uint64_t       m_SizeInBytes;

	ID3D12Heap*    m_pID3D12Heap;
	ID3D12Device4* m_pID3D12Device;

	CHeapAllocator m_Allocator;

public:
	CHeap(void);
	~CHeap(void);

	bool Initialize(ID3D12Device4* pID3D12Device, ID3D12Heap* pID3D12Heap);
	void Uninitialize(void);

	CAllocation* CreateAllocation(const D3D12_RESOURCE_DESC& pDesc, D3D12_RESOURCE_STATES InitialState);
	void         DeleteAllocation(CAllocation* pAllocation);
};

#endif // CG_CHEAP_HPP