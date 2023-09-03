#ifndef CG_COPY_CCOMMAND_BUFFER_HPP
#define CG_COPY_CCOMMAND_BUFFER_HPP

#include "CCommandBuffer.hpp"

struct D3D12_BOX;
struct D3D12_RESOURCE_BARRIER;
struct D3D12_TEXTURE_COPY_LOCATION;

struct ID3D12Resource;
struct ID3D12CommandAllocator;
struct ID3D12GraphicsCommandList;

class CCopyCommandBuffer : public CCommandBuffer
{
public:
	CCopyCommandBuffer(void);
	~CCopyCommandBuffer(void);

	virtual void ClearRenderBuffer(const RenderBuffer& rBuffer, const float RGBA[]);
	virtual void Present(const RenderBuffer& rBuffer);
	virtual void SetVertexBuffers(uint32_t NumBuffers, const IVertexBuffer* pIVertexBuffers);
	virtual void SetConstantBuffer(uint32_t Index, IConstantBuffer* pIConstantBuffer);
	virtual void Draw(uint32_t NumVertices);
	virtual void SetViewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h, float min_depth, float max_depth);
	virtual void ProgramPipeline(IRendererState* pIRendererState);
	virtual void SetRenderTarget(const RenderBuffer& rBuffer);

	void CopyResource(ID3D12Resource* pDstResource, ID3D12Resource* pSrcResource);
	void ResourceBarrier(uint32_t NumBarriers, const D3D12_RESOURCE_BARRIER * pBarriers);
	void CopyTextureRegion(const D3D12_TEXTURE_COPY_LOCATION* pDst, uint32_t DstX, uint32_t DstY, uint32_t DstZ, const D3D12_TEXTURE_COPY_LOCATION* pSrc, const D3D12_BOX* pSrcBox);
};

#endif // CG_COPY_CCOMMAND_BUFFER_HPP
