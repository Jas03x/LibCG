#include "CVertexBuffer.hpp"

#include "Windows.h"

#include <d3d12.h>

CVertexBuffer::CVertexBuffer(void)
{
	m_VertexBufferGpuVA = 0;
	m_pID3D12Resource = nullptr;

	ZeroMemory(&m_Desc, sizeof(VERTEX_BUFFER_DESC));
}

CVertexBuffer::~CVertexBuffer(void)
{

}

bool CVertexBuffer::Initialize(ID3D12Resource* pID3D12VertexBuffer, const VERTEX_BUFFER_DESC& rDesc)
{
	bool status = true;

	if (pID3D12VertexBuffer != nullptr)
	{
		m_pID3D12Resource = pID3D12VertexBuffer;
		m_VertexBufferGpuVA = m_pID3D12Resource->GetGPUVirtualAddress();
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

	if (m_pID3D12Resource != nullptr)
	{
		m_pID3D12Resource->Release();
		m_pID3D12Resource = nullptr;
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
