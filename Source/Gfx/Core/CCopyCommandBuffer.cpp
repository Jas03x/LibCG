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
