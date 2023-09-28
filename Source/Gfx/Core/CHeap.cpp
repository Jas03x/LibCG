#include "CHeap.hpp"

#include <d3d12.h>

#include "Cg.hpp"

#include "CAllocation.hpp"

CHeap::CHeap(void)
{
	m_SizeInBytes = 0;

	m_pID3D12Heap = nullptr;
	m_pID3D12Device = nullptr;
}

CHeap::~CHeap(void)
{
	CgAssert(m_pID3D12Heap == nullptr, L"DX12 heap interface not released\n");
	CgAssert(m_pID3D12Device == nullptr, L"DX12 device interface not released\n");
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
	else
	{
		status = false;
	}

	if (status)
	{
		status = m_Allocator.Initialize(m_SizeInBytes);
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

CAllocation* CHeap::CreateAllocation(const D3D12_RESOURCE_DESC& rDesc, D3D12_RESOURCE_STATES InitialState)
{
	bool status = true;

	uint64_t Size = 0;
	uint64_t Offset = 0;

	CAllocation* pAllocation = nullptr;
	ID3D12Resource* pID3D12Resource = nullptr;

	switch (rDesc.Dimension)
	{
		case D3D12_RESOURCE_DIMENSION_BUFFER:
		{
			Size = rDesc.Width;
			break;
		}
		case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
		{
			Size = rDesc.Width * rDesc.Height;
			break;
		}
		default:
		{
			status = false;
			Console::Write(L"Error: unknown allocation type\n");
			break;
		}
	}

	if (status)
	{
		if (!m_Allocator.Allocate(Size, rDesc.Alignment, Offset))
		{
			status = false;
		}
	}

	if (status)
	{
		if (m_pID3D12Device->CreatePlacedResource(m_pID3D12Heap, Offset, &rDesc, InitialState, NULL, __uuidof(ID3D12Resource), reinterpret_cast<void**>(&pID3D12Resource)) != S_OK)
		{
			status = false;
		}
	}

	if (status)
	{
		pAllocation = new CAllocation();

		if (pAllocation != nullptr)
		{
			if (!pAllocation->Initialize(Offset, pID3D12Resource))
			{
				status = false;
				delete pAllocation;
				pAllocation = nullptr;
			}
		}
		else
		{
			status = false;
		}
	}

	return pAllocation;
}

void CHeap::DeleteAllocation(CAllocation* pAllocation)
{
	if (pAllocation != nullptr)
	{
		m_Allocator.Free(pAllocation->GetOffset());

		delete pAllocation;
	}
}
