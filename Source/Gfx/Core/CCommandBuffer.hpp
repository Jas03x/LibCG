#ifndef CG_CCOMMAND_BUFFER_HPP
#define CG_CCOMMAND_BUFFER_HPP

#include "CgGfx.hpp"

struct ID3D12CommandAllocator;
struct ID3D12GraphicsCommandList;

class CCommandBuffer : public ICommandBuffer
{
private:
	enum STATE
	{
		STATE_ERROR     = 0,
		STATE_RESET     = 1,
		STATE_RECORDING = 2,
		STATE_CLOSED    = 3
	};

	COMMAND_BUFFER_TYPE        m_Type;

	STATE                      m_State;

	ID3D12CommandAllocator*    m_pID3D12CommandAllocator;
	ID3D12GraphicsCommandList* m_pID3D12CommandList;

public:
	CCommandBuffer(void);
	~CCommandBuffer(void);

	bool                       Initialize(COMMAND_BUFFER_TYPE Type, ID3D12CommandAllocator* pICommandAllocator, ID3D12GraphicsCommandList* pICommandList);
	void                       Uninitialize(void);

	virtual void               ClearRenderBuffer(const RenderBuffer& rBuffer, const float RGBA[]);
	virtual void               Present(const RenderBuffer& rBuffer);
	virtual void               SetVertexBuffers(uint32_t NumBuffers, const IVertexBuffer* pIVertexBuffers);
	virtual void               SetConstantBuffer(uint32_t Index, IConstantBuffer* pIConstantBuffer);
	virtual void			   Draw(uint32_t NumVertices);
	virtual void               SetViewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h, float min_depth, float max_depth);
	virtual void               ProgramPipeline(IRendererState* pIRendererState);
	virtual void               SetRenderTarget(const RenderBuffer& rBuffer);

	virtual bool               Finalize(void);
	virtual bool               Reset(void);

	COMMAND_BUFFER_TYPE        GetType(void);
	ID3D12GraphicsCommandList* GetD3D12Interface(void);
};

#endif // CG_CCOMMAND_BUFFER_HPP
