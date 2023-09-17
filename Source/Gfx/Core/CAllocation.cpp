#include "CAllocation.hpp"

#include <d3d12.h>

CAllocation::CAllocation(void)
{
	m_Offset = 0;
	m_pID3D12Resource = nullptr;
}

CAllocation::~CAllocation(void)
{

}

bool CAllocation::Initialize(uint64_t Offset, ID3D12Resource* pID3D12Resource)
{
	bool status = true;

	if (pID3D12Resource != nullptr)
	{
		m_Offset = Offset;
		m_pID3D12Resource = pID3D12Resource;
	}

	return status;
}

void CAllocation::Uninitialize(void)
{
	m_Offset = 0;

	if (m_pID3D12Resource != nullptr)
	{
		m_pID3D12Resource->Release();
		m_pID3D12Resource = nullptr;
	}
}

uint64_t CAllocation::GetOffset(void)
{
	return m_Offset;
}

ID3D12Resource* CAllocation::GetD3D12Interface(void)
{
	return m_pID3D12Resource;
}
