#ifndef CG_CCOMMAND_BUFFER_HPP
#define CG_CCOMMAND_BUFFER_HPP

#include "CgGfx.hpp"

struct ID3D12CommandAllocator;
struct ID3D12GraphicsCommandList;

class CCommandBuffer : public ICommandBuffer
{
protected:
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

	bool                       Initialize(ID3D12CommandAllocator* pICommandAllocator, ID3D12GraphicsCommandList* pICommandList);
	void                       Uninitialize(void);

	virtual bool               Finalize(void);
	virtual bool               Reset(IRendererState* pIRendererState);

	COMMAND_BUFFER_TYPE        GetType(void);
	ID3D12GraphicsCommandList* GetD3D12Interface(void);
};

#endif // CG_CCOMMAND_BUFFER_HPP
