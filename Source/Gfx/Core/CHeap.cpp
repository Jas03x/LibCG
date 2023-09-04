#include "CHeap.hpp"

#include <d3d12.h>

#include "Cg.hpp"

CHeap::CHeap(void)
{
	m_SizeInBytes = 0;
	m_OffsetInBytes = 0;

	m_pID3D12Heap = nullptr;
	m_pID3D12Device = nullptr;
}

CHeap::~CHeap(void)
{

}

bool CHeap::Initialize(ID3D12Device4* pID3D12Device, ID3D12Heap* pID3D12Heap)
{
	bool status = true;

	if ((pID3D12Device != nullptr) && (pID3D12Heap != nullptr))
	{
		D3D12_HEAP_DESC Desc = pID3D12Heap->GetDesc();
		m_SizeInBytes = Desc.SizeInBytes;

		m_pID3D12Heap = pID3D12Heap;
		m_pID3D12Device = pID3D12Device;
	}

	return status;
}

void CHeap::Uninitialize(void)
{
	if (m_pID3D12Heap != nullptr)
	{
		m_pID3D12Heap->Release();
		m_pID3D12Heap = nullptr;
	}

	m_pID3D12Device = nullptr;
}

bool CHeap::AllocateResource(const D3D12_RESOURCE_DESC& pDesc, D3D12_RESOURCE_STATES InitialState, ID3D12Resource** pResource)
{
	bool status = true;

	uint64_t AllocationSize = pDesc.Width;
	uint64_t AllocationOffset = (m_OffsetInBytes & ~(pDesc.Alignment - 1));

	if (m_OffsetInBytes + AllocationSize >= m_SizeInBytes)
	{
		status = false;
		Console::Write(L"Error: Cannot fit allocation in heap\n");
	}

	if (status)
	{
		if (m_pID3D12Device->CreatePlacedResource(m_pID3D12Heap, m_OffsetInBytes, &pDesc, InitialState, NULL, __uuidof(ID3D12Resource), reinterpret_cast<void**>(pResource)) == S_OK)
		{
			m_OffsetInBytes += AllocationSize;
		}
		else
		{
			status = false;
		}
	}

	return status;
}
