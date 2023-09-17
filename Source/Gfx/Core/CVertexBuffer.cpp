#include "CVertexBuffer.hpp"

#include "Windows.h"

#include <d3d12.h>

#include "CAllocation.hpp"

CVertexBuffer::CVertexBuffer(void)
{
	m_VertexBufferGpuVA = 0;
	m_pAllocation = nullptr;

	ZeroMemory(&m_Desc, sizeof(VERTEX_BUFFER_DESC));
}

CVertexBuffer::~CVertexBuffer(void)
{

}

bool CVertexBuffer::Initialize(CAllocation* pAllocation, const VERTEX_BUFFER_DESC& rDesc)
{
	bool status = true;

	if (pAllocation != nullptr)
	{
		m_pAllocation = pAllocation;
		m_VertexBufferGpuVA = pAllocation->GetD3D12Interface()->GetGPUVirtualAddress();
		CopyMemory(&m_Desc, &rDesc, sizeof(VERTEX_BUFFER_DESC));
	}
	else
	{
		status = false;
	}

	return status;
}

void CVertexBuffer::Uninitialize(void)
{
	m_VertexBufferGpuVA = 0;
	ZeroMemory(&m_Desc, sizeof(VERTEX_BUFFER_DESC));

	if (m_pAllocation != nullptr)
	{
		m_pAllocation->Uninitialize();
		delete m_pAllocation;
		m_pAllocation = nullptr;
	}
}

UINT64 CVertexBuffer::GetGpuVA(void)
{
	return m_VertexBufferGpuVA;
}

const VERTEX_BUFFER_DESC& CVertexBuffer::GetDesc(void) const
{
	return m_Desc;
}
