#include "CCopyCommandBuffer.hpp"

#include <d3d12.h>

#include "Cg.hpp"

CCopyCommandBuffer::CCopyCommandBuffer(void)
{
	m_Type = COMMAND_BUFFER_TYPE_COPY;
}

CCopyCommandBuffer::~CCopyCommandBuffer(void)
{

}

void CCopyCommandBuffer::ClearRenderBuffer(const RenderBuffer& rBuffer, const float RGBA[])
{
	m_State = STATE_ERROR;
	Console::Write(L"Error: Invalid graphics command buffer call\n");
}

void CCopyCommandBuffer::Present(const RenderBuffer& rBuffer)
{
	m_State = STATE_ERROR;
	Console::Write(L"Error: Invalid graphics command buffer call\n");
}

void CCopyCommandBuffer::SetVertexBuffers(uint32_t NumBuffers, const IVertexBuffer* pIVertexBuffers)
{
	m_State = STATE_ERROR;
	Console::Write(L"Error: Invalid graphics command buffer call\n");
}

void CCopyCommandBuffer::SetConstantBuffer(uint32_t Index, IConstantBuffer* pIConstantBuffer)
{
	m_State = STATE_ERROR;
	Console::Write(L"Error: Invalid graphics command buffer call\n");
}

void CCopyCommandBuffer::Draw(uint32_t NumVertices)
{
	m_State = STATE_ERROR;
	Console::Write(L"Error: Invalid graphics command buffer call\n");
}

void CCopyCommandBuffer::SetViewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h, float min_depth, float max_depth)
{
	m_State = STATE_ERROR;
	Console::Write(L"Error: Invalid graphics command buffer call\n");
}

void CCopyCommandBuffer::ProgramPipeline(IRendererState* pIRendererState)
{
	m_State = STATE_ERROR;
	Console::Write(L"Error: Invalid graphics command buffer call\n");
}

void CCopyCommandBuffer::SetRenderTarget(const RenderBuffer& rBuffer)
{
	m_State = STATE_ERROR;
	Console::Write(L"Error: Invalid graphics command buffer call\n");
}

void CCopyCommandBuffer::CopyResource(ID3D12Resource* pDstResource, ID3D12Resource* pSrcResource)
{
	if ((m_State != STATE_CLOSED) && (m_State != STATE_ERROR))
	{
		if ((pDstResource != nullptr) && (pSrcResource != nullptr))
		{
			m_State = STATE_RECORDING;
			m_pID3D12CommandList->CopyResource(pDstResource, pSrcResource);
		}
		else
		{
			Console::Write(L"Error: Invalid copy resource parameter(s)\n");
			m_State = STATE_ERROR;
		}
	}
}

void CCopyCommandBuffer::ResourceBarrier(uint32_t NumBarriers, const D3D12_RESOURCE_BARRIER* pBarriers)
{
	if ((m_State != STATE_CLOSED) && (m_State != STATE_ERROR))
	{
		if ((NumBarriers > 0) && (pBarriers != nullptr))
		{
			m_State = STATE_RECORDING;
			m_pID3D12CommandList->ResourceBarrier(NumBarriers, pBarriers);
		}
		else
		{
			Console::Write(L"Error: Invalid resource barrier parameter(s)\n");
			m_State = STATE_ERROR;
		}
	}
}

void CCopyCommandBuffer::CopyTextureRegion(const D3D12_TEXTURE_COPY_LOCATION* pDst, uint32_t DstX, uint32_t DstY, uint32_t DstZ, const D3D12_TEXTURE_COPY_LOCATION* pSrc, const D3D12_BOX* pSrcBox)
{
	if ((m_State != STATE_CLOSED) && (m_State != STATE_ERROR))
	{
		if ((pDst != nullptr) && (pSrc != nullptr))
		{
			m_State = STATE_RECORDING;
			m_pID3D12CommandList->CopyTextureRegion(pDst, DstX, DstY, DstZ, pSrc, pSrcBox);
		}
		else
		{
			Console::Write(L"Error: Invalid copy texture region parameter(s)\n");
			m_State = STATE_ERROR;
		}
	}
}
