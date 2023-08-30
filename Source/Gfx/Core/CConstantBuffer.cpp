#include "CConstantBuffer.hpp"

#include "Cg.hpp"

#include <d3d12.h>

CConstantBuffer::CConstantBuffer(void)
{
	m_CpuVA = NULL;
	m_GpuVA = 0;
	m_pID3D12Resource = NULL;
}

CConstantBuffer::~CConstantBuffer(void)
{

}

bool CConstantBuffer::Initialize(ID3D12Resource* pID3D12ConstantBufferResource, void* CpuVA)
{
	bool status = true;

	if (pID3D12ConstantBufferResource != NULL)
	{
		m_CpuVA = CpuVA;
		m_GpuVA = pID3D12ConstantBufferResource->GetGPUVirtualAddress();
		m_pID3D12Resource = pID3D12ConstantBufferResource;
	}
	else
	{
		status = false;
		Console::Write(L"Error: Failed to initialize constant buffer\n");
	}

	return status;
}

void CConstantBuffer::Uninitialize(void)
{
	if (m_pID3D12Resource != NULL)
	{
		m_pID3D12Resource->Release();
		m_pID3D12Resource = NULL;
	}
}

void* CConstantBuffer::GetCpuVA(void)
{
	return m_CpuVA;
}

UINT64 CConstantBuffer::GetGpuVA(void)
{
	return m_GpuVA;
}
