#ifndef CG_CGFX_DEVICE_HPP
#define CG_CGFX_DEVICE_HPP

#include "CgGfx.hpp"

struct IDXGIDebug;
struct IDXGIFactory7;
struct IDXGIAdapter4;
struct IDXGISwapChain4;

struct ID3D12Debug;
struct ID3D12Device;
struct ID3D12Resource;
struct ID3D12Heap;
struct ID3D12DescriptorHeap;

class CCommandQueue;
class CSwapChain;

typedef struct HINSTANCE__* HMODULE;

class CGfxContext : public IGfxContext
{
private:
	enum : uint32_t { ADAPTER_INDEX__INVALID = 0xFFFFFFFF };
	enum : uint32_t { MAX_INPUT_ELEMENTS = 32 };

	IWindow*                   m_pIWindow;

#if _DEBUG
	HMODULE					   m_hDxgiDebugModule;

	IDXGIDebug*				   m_pIDxgiDebugInterface;
	ID3D12Debug*			   m_pID3D12DebugInterface;
#endif

	IDXGIFactory7*			   m_pIDxgiFactory;
	IDXGIAdapter4*			   m_pIDxgiAdapter;

	ID3D12Device*			   m_pID3D12Device;
	
	ID3D12Heap*				   m_pID3D12UploadHeap;
	ID3D12Heap*				   m_pID3D12PrimaryHeap;

	ID3D12DescriptorHeap*      m_pID3D12ShaderResourceHeap;

	CCommandQueue*             m_pCopyQueue;
	CCommandQueue*             m_pGraphicsQueue;

	CSwapChain*                m_pSwapChain;

	ICommandBuffer*            m_pICopyCommandBuffer;

public:
	CGfxContext(void);
	virtual ~CGfxContext(void);

	bool				       Initialize(IWindow* pIWindow, const ContextFactory::Descriptor& rDesc);
	void				       Uninitialize(void);

private:
	bool				       EnumerateDxgiAdapters(void);
	bool				       PrintAdapterProperties(uint32_t uIndex, IDXGIAdapter4* pIAdapter);
	bool				       PrintDeviceProperties(void);

	bool                       InitializeHeaps(const ContextFactory::Descriptor& rDesc);
	bool                       InitializeDescriptorHeaps(void);
	bool                       InitializeSwapChain(void);

	ICommandQueue*             CreateCommandQueue(COMMAND_QUEUE_TYPE Type);
	void                       DestroyCommandQueue(ICommandQueue* pICommandQueue);

public:
	virtual ICommandBuffer*    CreateCommandBuffer(COMMAND_BUFFER_TYPE Type);
	virtual void               DestroyCommandBuffer(ICommandBuffer* pICommandBuffer);

	virtual IRendererState*    CreateRendererState(const RENDERER_STATE_DESC& rDesc);
	virtual void               DestroyRendererState(IRendererState* pIRendererState);

	virtual IConstantBuffer*   CreateConstantBuffer(const CONSTANT_BUFFER_DESC& rDesc);
	virtual void               DestroyConstantBuffer(IConstantBuffer* pIConstantBuffer);

	virtual IVertexBuffer*     CreateVertexBuffer(const void* pVertexData, uint32_t Size, uint32_t Stride);
	virtual void               DestroyVertexBuffer(IVertexBuffer* pIVertexBuffer);

	virtual IMesh*             CreateMesh(const MESH_DESC& rDesc);
	virtual void               DestroyMesh(IMesh* pIMesh);

	virtual ITexture*          CreateTexture(const TEXTURE_DESC& rDesc);
	virtual void               DestroyTexture(ITexture* pITexture);

	virtual bool               SubmitCommandBuffer(ICommandBuffer* pICommandBuffer);
	virtual bool               SyncQueue(COMMAND_QUEUE_TYPE Type);
};

#endif // CG_CGFX_DEVICE_HPP
