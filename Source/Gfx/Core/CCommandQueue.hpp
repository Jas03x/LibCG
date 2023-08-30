#ifndef CG_CCOMMAND_QUEUE_HPP
#define CG_CCOMMAND_QUEUE_HPP

#include "CgDef.hpp"
#include "CgGfx.hpp"

struct ID3D12CommandQueue;
struct ID3D12CommandAllocator;
struct ID3D12Fence;

class CCommandQueue : public ICommandQueue
{
private:
	enum : uint32_t { COMMAND_QUEUE_TIMEOUT = 1000 };

	COMMAND_QUEUE_TYPE      m_Type;

	ID3D12CommandQueue*     m_pID3D12CommandQueue;
	ID3D12Fence*            m_pID3D12Fence;

	HANDLE                  m_hFenceEvent;
	uint64_t                m_FenceValue;

public:
	CCommandQueue(void);
	~CCommandQueue(void);

	bool                    Initialize(COMMAND_QUEUE_TYPE Type, ID3D12CommandQueue* pICommandQueue, ID3D12Fence* pIFence);
	void                    Uninitialize(void);

	bool                    SubmitCommandBuffer(ICommandBuffer* pICommandBuffer);
	bool                    Sync(void);

	ID3D12CommandQueue*     GetD3D12CommandQueue(void);
};

#endif // CG_CCOMMAND_QUEUE_HPP
