#include "CCommandBuffer.hpp"

#include <d3d12.h>

#include "Cg.hpp"

#include "CRendererState.hpp"

CCommandBuffer::CCommandBuffer(void)
{
	m_Type = COMMAND_BUFFER_TYPE_INVALID;
	m_pID3D12CommandAllocator = nullptr;
	m_pID3D12CommandList = nullptr;
	m_State = STATE_ERROR;
}

CCommandBuffer::~CCommandBuffer(void)
{

}

bool CCommandBuffer::Initialize(ID3D12CommandAllocator* pICommandAllocator, ID3D12GraphicsCommandList* pICommandList)
{
	bool status = true;

	m_State = STATE_CLOSED;

	if ((pICommandAllocator != nullptr) && (pICommandList != nullptr))
	{
		m_pID3D12CommandAllocator = pICommandAllocator;
		m_pID3D12CommandList = pICommandList;
	}
	else
	{
		status = false;
		Console::Write(L"Error: Could not create command buffer - null interfaces\n");
	}

	return status;
}

void CCommandBuffer::Uninitialize(void)
{
	if (m_pID3D12CommandList != nullptr)
	{
		m_pID3D12CommandList->Release();
		m_pID3D12CommandList = nullptr;
	}

	if (m_pID3D12CommandAllocator != nullptr)
	{
		m_pID3D12CommandAllocator->Release();
		m_pID3D12CommandAllocator = nullptr;
	}
}

COMMAND_BUFFER_TYPE CCommandBuffer::GetType(void)
{
	return m_Type;
}

ID3D12GraphicsCommandList* CCommandBuffer::GetD3D12Interface(void)
{
	return m_pID3D12CommandList;
}

bool CCommandBuffer::Finalize(void)
{
	bool status = true;

	if (m_State == STATE_RECORDING)
	{
		if (m_pID3D12CommandList->Close() == S_OK)
		{
			m_State = STATE_CLOSED;
		}
		else
		{
			status = false;
			Console::Write(L"Error: Could not finalize command list\n");
		}
	}
	else
	{
		status = false;
		Console::Write(L"Error: Cannot finalize empty command buffer\n");
	}

	return status;
}

bool CCommandBuffer::Reset(IRendererState* pIRendererState)
{
	bool status = true;

	if (m_State != STATE_RESET)
	{
		if (m_State == STATE_CLOSED)
		{
			if (m_pID3D12CommandAllocator->Reset() != S_OK)
			{
				status = false;
				Console::Write(L"Error: Failed to reset D3D12 command allocator\n");
			}

			if (status)
			{
				ID3D12PipelineState* pID3D12PipelineState = nullptr;

				if (pIRendererState != nullptr)
				{
					pID3D12PipelineState = static_cast<CRendererState*>(pIRendererState)->GetD3D12PipelineState();
				}

				if (m_pID3D12CommandList->Reset(m_pID3D12CommandAllocator, pID3D12PipelineState) == S_OK)
				{
					m_State = STATE_RESET;
				}
				else
				{
					status = false;
					Console::Write(L"Error: Failed to reset D3D12 command list\n");
				}
			}
		}
		else
		{
			status = false;
			Console::Write(L"Error: Command buffer to be reset needs to be in a closed state\n");
		}
	}

	return status;
}
