#ifndef CG_CSWAPCHAIN_HPP
#define CG_CSWAPCHAIN_HPP

#include "CgDef.hpp"
#include "CgGfx.hpp"

struct IDXGISwapChain4;

struct ID3D12Resource;
struct ID3D12DescriptorHeap;

class CSwapChain
{
public:
	enum : uint32_t { NUM_BUFFERS = 2 };

	struct Descriptor
	{
		uint32_t     RtvDescriptorIncrement;
		RenderBuffer RenderBuffers[NUM_BUFFERS];
	};

private:
	uint32_t              m_FrameIndex;

	IDXGISwapChain4*      m_pIDxgiSwapChain;

	ID3D12DescriptorHeap* m_pID3D12DescriptorHeap;
	ID3D12Resource*       m_pID3D12RenderBuffers[NUM_BUFFERS];
	
	uint32_t              m_RtvDescriptorIncrement;
	uint64_t			  m_RenderBufferCpuDescriptors[NUM_BUFFERS];

public:
	CSwapChain(void);
	~CSwapChain(void);

	bool Initialize(IDXGISwapChain4* pIDxgiSwapChain, ID3D12DescriptorHeap* pIRtvDescriptorHeap, const Descriptor& rDesc);
	void Uninitialize(void);

	uint32_t GetNumBuffers(void);
	void     GetCurrentRenderBuffer(HANDLE& hResource, uint64_t& CpuDescriptor);
	bool     Present(void);
};

#endif // CG_CSWAPCHAIN_HPP