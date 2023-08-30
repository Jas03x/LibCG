#include "CSwapChain.hpp"

#include <d3d12.h>
#include <dxgi1_6.h>

#include "Cg.hpp"

CSwapChain::CSwapChain(void)
{
	m_FrameIndex = 0;
	m_RtvDescriptorIncrement = 0;

	m_pIDxgiSwapChain = nullptr;
	m_pID3D12DescriptorHeap = nullptr;

	for (uint32_t i = 0; i < NUM_BUFFERS; i++)
	{
		m_pID3D12RenderBuffers[i] = nullptr;
		m_RenderBufferCpuDescriptors[i] = 0;
	}
}

CSwapChain::~CSwapChain(void)
{

}

bool CSwapChain::Initialize(IDXGISwapChain4* pIDxgiSwapChain, ID3D12DescriptorHeap* pIRtvDescriptorHeap, const Descriptor& rDesc)
{
	bool status = true;

	if ((pIDxgiSwapChain != nullptr) && (pIRtvDescriptorHeap != nullptr))
	{
		m_pIDxgiSwapChain = pIDxgiSwapChain;
		m_pID3D12DescriptorHeap = pIRtvDescriptorHeap;

		for (uint32_t i = 0; i < NUM_BUFFERS; i++)
		{
			m_RenderBufferCpuDescriptors[i] = rDesc.RenderBuffers[i].CpuDescriptor;
			m_pID3D12RenderBuffers[i] = reinterpret_cast<ID3D12Resource*>(rDesc.RenderBuffers[i].hResource);
		}

		m_RtvDescriptorIncrement = rDesc.RtvDescriptorIncrement;
		m_FrameIndex = m_pIDxgiSwapChain->GetCurrentBackBufferIndex();
	}
	else
	{
		status = false;
		Console::Write(L"Error: NULL swapchain parameters\n");
	}

	return status;
}

void CSwapChain::Uninitialize(void)
{
	for (uint32_t i = 0; i < NUM_BUFFERS; i++)
	{
		if (m_pID3D12RenderBuffers[i] != nullptr)
		{
			m_pID3D12RenderBuffers[i]->Release();
			m_pID3D12RenderBuffers[i] = nullptr;
		}
	}

	if (m_pID3D12DescriptorHeap != nullptr)
	{
		m_pID3D12DescriptorHeap->Release();
		m_pID3D12DescriptorHeap = nullptr;
	}

	if (m_pIDxgiSwapChain != nullptr)
	{
		m_pIDxgiSwapChain->Release();
		m_pIDxgiSwapChain = nullptr;
	}
}

uint32_t CSwapChain::GetNumBuffers(void)
{
	return NUM_BUFFERS;
}

void CSwapChain::GetCurrentRenderBuffer(HANDLE& hResource, uint64_t& CpuDescriptor)
{
	hResource = m_pID3D12RenderBuffers[m_FrameIndex];
	CpuDescriptor = m_RenderBufferCpuDescriptors[m_FrameIndex];
}

bool CSwapChain::Present(void)
{
	bool status = true;

	if (m_pIDxgiSwapChain->Present(1, 0) == S_OK)
	{
		m_FrameIndex = (m_FrameIndex + 1) % NUM_BUFFERS;
	}
	else
	{
		status = false;
		Console::Write(L"Error: Failed to signal swap chain to present\n");
	}

	return status;
}
