#ifndef CG_GFX_CCOMMAND_BUFFER_HPP
#define CG_GFX_CCOMMAND_BUFFER_HPP

#include "CCommandBuffer.hpp"

struct ID3D12CommandAllocator;
struct ID3D12GraphicsCommandList;

class CGfxCommandBuffer : public CCommandBuffer
{
public:
	CGfxCommandBuffer(void);
	~CGfxCommandBuffer(void);

	virtual void ClearRenderBuffer(const RenderBuffer& rBuffer, const float RGBA[]);
	virtual void Present(const RenderBuffer& rBuffer);
	virtual void SetVertexBuffers(uint32_t NumBuffers, const IVertexBuffer* pIVertexBuffers);
	virtual void SetConstantBuffer(uint32_t Index, IConstantBuffer* pIConstantBuffer);
	virtual void Draw(uint32_t NumVertices);
	virtual void SetViewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h, float min_depth, float max_depth);
	virtual void ProgramPipeline(IRendererState* pIRendererState);
	virtual void SetRenderTarget(const RenderBuffer& rBuffer);

	virtual bool Reset(IRendererState* pIRendererState) override;
};

#endif // CG_GFX_CCOMMAND_BUFFER_HPP
