#include "CGfxCommandBuffer.hpp"

#include <d3d12.h>

#include "CConstantBuffer.hpp"
#include "CMesh.hpp"
#include "CRendererState.hpp"

CGfxCommandBuffer::CGfxCommandBuffer(void)
{
	m_Type = COMMAND_BUFFER_TYPE_GRAPHICS;
}

CGfxCommandBuffer::~CGfxCommandBuffer(void)
{

}

void CGfxCommandBuffer::SetViewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h, float min_depth, float max_depth)
{
	if ((m_State != STATE_CLOSED) && (m_State != STATE_ERROR))
	{
		D3D12_VIEWPORT Viewport = {};
		Viewport.TopLeftX = static_cast<float>(x);
		Viewport.TopLeftY = static_cast<float>(y);
		Viewport.Width = static_cast<float>(w);
		Viewport.Height = static_cast<float>(h);
		Viewport.MinDepth = min_depth;
		Viewport.MaxDepth = max_depth;

		D3D12_RECT ScissorRect = {};
		ScissorRect.left = x;
		ScissorRect.top = y;
		ScissorRect.right = w;
		ScissorRect.bottom = h;

		m_State = STATE_RECORDING;
		m_pID3D12CommandList->RSSetViewports(1, &Viewport);
		m_pID3D12CommandList->RSSetScissorRects(1, &ScissorRect);
	}
}

void CGfxCommandBuffer::ProgramPipeline(IRendererState* pIRendererState)
{
	if ((m_State != STATE_CLOSED) && (m_State != STATE_ERROR))
	{
		if (pIRendererState != nullptr)
		{
			CRendererState* pRendererState = static_cast<CRendererState*>(pIRendererState);
			ID3D12DescriptorHeap* pHeaps[] = { pRendererState->GetShaderResourceHeap() };

			m_State = STATE_RECORDING;
			m_pID3D12CommandList->SetPipelineState(pRendererState->GetD3D12PipelineState());
			m_pID3D12CommandList->SetGraphicsRootSignature(pRendererState->GetD3D12RootSignature());
			m_pID3D12CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // TODO: Remove this hard coded primitive type
			m_pID3D12CommandList->SetDescriptorHeaps(_countof(pHeaps), pHeaps); // TODO: Perhaps remove this unnecessary call - its only needed once
			m_pID3D12CommandList->SetGraphicsRootDescriptorTable(1, pRendererState->GetShaderResourceHeap()->GetGPUDescriptorHandleForHeapStart());
		}
		else
		{
			Console::Write(L"Error: Could not set command buffer renderer - received null renderer\n");
			m_State = STATE_ERROR;
		}
	}
}

void CGfxCommandBuffer::SetConstantBuffer(uint32_t Index, IConstantBuffer* pIConstantBuffer)
{
	if ((m_State != STATE_CLOSED) && (m_State != STATE_ERROR))
	{
		if (pIConstantBuffer != nullptr)
		{
			CConstantBuffer* pConstantBuffer = static_cast<CConstantBuffer*>(pIConstantBuffer);

			m_State = STATE_RECORDING;
			m_pID3D12CommandList->SetGraphicsRootConstantBufferView(0, pConstantBuffer->GetGpuVA());
		}
		else
		{
			Console::Write(L"Error: Could not set command buffer renderer - received null renderer\n");
			m_State = STATE_ERROR;
		}
	}
}

void CGfxCommandBuffer::SetRenderTarget(const RenderBuffer& rBuffer)
{
	if ((m_State != STATE_CLOSED) && (m_State != STATE_ERROR))
	{
		if ((rBuffer.hResource != nullptr) && (rBuffer.CpuDescriptor != 0))
		{
			D3D12_CPU_DESCRIPTOR_HANDLE CpuDescHandle = { rBuffer.CpuDescriptor };
			ID3D12Resource* pIRenderBuffer = reinterpret_cast<ID3D12Resource*>(rBuffer.hResource);

			D3D12_RESOURCE_BARRIER Barrier = {};
			Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			Barrier.Transition.pResource = pIRenderBuffer;
			Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
			Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

			m_State = STATE_RECORDING;
			m_pID3D12CommandList->ResourceBarrier(1, &Barrier);
			m_pID3D12CommandList->OMSetRenderTargets(1, &CpuDescHandle, FALSE, nullptr);
		}
		else
		{
			m_State = STATE_ERROR;
			Console::Write(L"Error: Could not set command buffer render target - received null render buffer\n");
		}
	}
}

void CGfxCommandBuffer::Present(const RenderBuffer& rBuffer)
{
	if ((m_State != STATE_CLOSED) && (m_State != STATE_ERROR))
	{
		if (rBuffer.hResource != nullptr)
		{
			ID3D12Resource* pIRenderBuffer = reinterpret_cast<ID3D12Resource*>(rBuffer.hResource);

			D3D12_RESOURCE_BARRIER Barrier = {};
			Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			Barrier.Transition.pResource = pIRenderBuffer;
			Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
			Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

			m_State = STATE_RECORDING;
			m_pID3D12CommandList->ResourceBarrier(1, &Barrier);
		}
		else
		{
			m_State = STATE_ERROR;
			Console::Write(L"Error: Could not present - received null render buffer\n");
		}
	}
}

void CGfxCommandBuffer::ClearRenderBuffer(const RenderBuffer& rBuffer, const float RGBA[])
{
	if ((m_State != STATE_CLOSED) && (m_State != STATE_ERROR))
	{
		if ((rBuffer.CpuDescriptor != 0) && (RGBA != nullptr))
		{
			D3D12_CPU_DESCRIPTOR_HANDLE CpuDescHandle = { rBuffer.CpuDescriptor };

			m_State = STATE_RECORDING;
			m_pID3D12CommandList->ClearRenderTargetView(CpuDescHandle, RGBA, 0, nullptr);
		}
		else
		{
			m_State = STATE_ERROR;
			Console::Write(L"Error: Could not clear render buffer - received invalid parameter(s)\n");
		}
	}
}

void CGfxCommandBuffer::SetVertexBuffers(uint32_t NumBuffers, const IVertexBuffer* pIVertexBuffers)
{
	enum { MAX_VERTEX_BUFFERS = 8 };

	if ((m_State != STATE_CLOSED) && (m_State != STATE_ERROR))
	{
		bool status = true;

		if (pIVertexBuffers == nullptr)
		{
			Console::Write(L"Error: Null descriptors\n");
			status = false;
		}

		if (status && (NumBuffers > MAX_VERTEX_BUFFERS))
		{
			Console::Write(L"Error: More vertex buffers than supported\n");
			status = false;
		}

		if (status)
		{
			D3D12_VERTEX_BUFFER_VIEW VertexBufferViews[MAX_VERTEX_BUFFERS] = {};

			for (uint32_t i = 0; i < NumBuffers; i++)
			{
				const VERTEX_BUFFER_DESC& Desc = pIVertexBuffers[i].GetDesc();

				VertexBufferViews[i].BufferLocation = Desc.GpuVA;
				VertexBufferViews[i].SizeInBytes = Desc.Size;
				VertexBufferViews[i].StrideInBytes = Desc.Stride;
			}

			m_State = STATE_RECORDING;
			m_pID3D12CommandList->IASetVertexBuffers(0, NumBuffers, VertexBufferViews);
		}
		else
		{
			Console::Write(L"Error: Failed to set vertex buffer(s)\n");
			m_State = STATE_ERROR;
		}
	}
}

void CGfxCommandBuffer::Draw(uint32_t NumVertices)
{
	if ((m_State != STATE_CLOSED) && (m_State != STATE_ERROR))
	{
		m_State = STATE_RECORDING;
		m_pID3D12CommandList->DrawInstanced(NumVertices, 1, 0, 0);
	}
}
