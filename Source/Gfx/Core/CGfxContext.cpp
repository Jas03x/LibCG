#include "CGfxContext.hpp"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>

#include <math.h>

#include "CgSystem.hpp"

#include "CCommandBuffer.hpp"
#include "CCommandQueue.hpp"
#include "CConstantBuffer.hpp"
#include "CMesh.hpp"
#include "CRendererState.hpp"
#include "CSwapChain.hpp"
#include "CTexture.hpp"
#include "CVertexBuffer.hpp"
#include "CWindow.hpp"

#include "EnumTranslator.hpp"

IGfxContext* ContextFactory::CreateInstance(IWindow* pIWindow, const ContextFactory::Descriptor& rDesc)
{
	CGfxContext* pContext = new CGfxContext();

	if (pContext != nullptr)
	{
		if (!pContext->Initialize(pIWindow, rDesc))
		{
			ContextFactory::DestroyInstance(pContext);
			pContext = nullptr;
		}
	}

	return pContext;
}

void ContextFactory::DestroyInstance(IGfxContext* pIContext)
{
	CGfxContext* pContext = static_cast<CGfxContext*>(pIContext);

	if (pContext != nullptr)
	{
		pContext->Uninitialize();
		delete pContext;
		pContext = nullptr;
	}
}

CGfxContext::CGfxContext(void)
{
#if _DEBUG
	m_hDxgiDebugModule = nullptr;

	m_pIDxgiDebugInterface = nullptr;
	m_pID3D12DebugInterface = nullptr;
#endif

	m_pIDxgiFactory = nullptr;
	m_pIDxgiAdapter = nullptr;

	m_pIWindow = nullptr;
	m_pSwapChain = nullptr;

	m_pID3D12Device = nullptr;

	m_pID3D12UploadHeap = nullptr;
	m_pID3D12PrimaryHeap = nullptr;
	m_pID3D12ShaderResourceHeap = nullptr;

	m_pCopyQueue = nullptr;
	m_pGraphicsQueue = nullptr;

	m_pICopyCommandBuffer = nullptr;
}

CGfxContext::~CGfxContext(void)
{

}

bool CGfxContext::Initialize(IWindow* pIWindow, const ContextFactory::Descriptor& rDesc)
{
	bool status = true;

	if (pIWindow != nullptr)
	{
		m_pIWindow = pIWindow;
	}
	else
	{
		status = false;
	}

#if _DEBUG
	if (status)
	{
		if (D3D12GetDebugInterface(__uuidof(ID3D12Debug), reinterpret_cast<void**>(&m_pID3D12DebugInterface)) == S_OK)
		{
			m_pID3D12DebugInterface->EnableDebugLayer();
		}
		else
		{
			status = false;
			Console::Write(L"Error: Failed to get dx12 debug interface\n");
		}
	}

	if (status)
	{
		m_hDxgiDebugModule = GetModuleHandle(L"dxgidebug.dll");
		HRESULT (*pfnDxgiGetDebugInterface)(REFIID, void**) = nullptr;

		if (m_hDxgiDebugModule != nullptr)
		{
			pfnDxgiGetDebugInterface = reinterpret_cast<HRESULT(*)(REFIID, void**)>(GetProcAddress(m_hDxgiDebugModule, "DXGIGetDebugInterface"));

			if (pfnDxgiGetDebugInterface == nullptr)
			{
				Console::Write(L"Error: Could not find function DXGIGetDebugInterface in the dxgi debug module\n");
				status = false;
			}
		}
		else
		{
			Console::Write(L"Error: Could not load the dxgi debug module\n");
			status = false;
		}

		if (status)
		{
			if (pfnDxgiGetDebugInterface(__uuidof(IDXGIDebug), reinterpret_cast<void**>(&m_pIDxgiDebugInterface)) != S_OK)
			{
				Console::Write(L"Error: Failed to get dxgi debug interface\n");
				status = false;
			}
		}
	}
#endif

	if (status)
	{
		UINT Flags = 0;

#if _DEBUG
		Flags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

		if (CreateDXGIFactory2(Flags, __uuidof(IDXGIFactory7), reinterpret_cast<void**>(&m_pIDxgiFactory)) != S_OK)
		{
			status = false;
			Console::Write(L"Error: Failed to create dxgi factory\n");
		}
	}

	if (status)
	{
		status = EnumerateDxgiAdapters();

		if (!status)
		{
			status = false;
			Console::Write(L"Error: Could not get dxgi adapter\n");
		}
	}

	if (status)
	{
		if (D3D12CreateDevice(m_pIDxgiAdapter, D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), reinterpret_cast<void**>(&m_pID3D12Device)) != S_OK)
		{
			status = false;
		}

		if (m_pID3D12Device != nullptr)
		{
			PrintDeviceProperties();
		}
		else
		{
			status = false;
			Console::Write(L"Error: Could not create a DX12 device\n");
		}
	}

	if (status)
	{
		m_pCopyQueue = static_cast<CCommandQueue*>(CreateCommandQueue(COMMAND_QUEUE_TYPE_COPY));

		if (m_pCopyQueue == nullptr)
		{
			status = false;
			Console::Write(L"Error: Failed to create copy command queue\n");
		}
	}

	if (status)
	{
		m_pGraphicsQueue = static_cast<CCommandQueue*>(CreateCommandQueue(COMMAND_QUEUE_TYPE_GRAPHICS));

		if (m_pGraphicsQueue == nullptr)
		{
			status = false;
			Console::Write(L"Error: Failed to create graphics command queue\n");
		}
	}

	if (status)
	{
		status = InitializeSwapChain();
	}

	if (status)
	{
		status = InitializeHeaps(rDesc);
	}

	if (status)
	{
		status = InitializeDescriptorHeaps();
	}

	if (status)
	{
		m_pICopyCommandBuffer = CreateCommandBuffer(COMMAND_BUFFER_TYPE_COPY);

		if (m_pICopyCommandBuffer == nullptr)
		{
			status = false;
			Console::Write(L"Error: Failed to create copy command buffer\n");
		}
	}

	return status;
}

void CGfxContext::Uninitialize(void)
{
	if (m_pIWindow != nullptr)
	{
		static_cast<CWindow*>(m_pIWindow)->SwapChainNotification(CWindow::SWAPCHAIN_DESTROYED, NULL);
	}

	if (m_pICopyCommandBuffer != nullptr)
	{
		DestroyCommandBuffer(m_pICopyCommandBuffer);
		m_pICopyCommandBuffer = nullptr;
	}

	if (m_pID3D12ShaderResourceHeap != nullptr)
	{
		m_pID3D12ShaderResourceHeap->Release();
		m_pID3D12ShaderResourceHeap = nullptr;
	}

	if (m_pID3D12PrimaryHeap != nullptr)
	{
		m_pID3D12PrimaryHeap->Release();
		m_pID3D12PrimaryHeap = nullptr;
	}

	if (m_pID3D12UploadHeap != nullptr)
	{
		m_pID3D12UploadHeap->Release();
		m_pID3D12UploadHeap = nullptr;
	}

	if (m_pCopyQueue != nullptr)
	{
		DestroyCommandQueue(m_pCopyQueue);
		m_pCopyQueue = nullptr;
	}

	if (m_pGraphicsQueue != nullptr)
	{
		DestroyCommandQueue(m_pGraphicsQueue);
		m_pGraphicsQueue = nullptr;
	}

	if (m_pSwapChain != nullptr)
	{
		m_pSwapChain->Uninitialize();
		delete m_pSwapChain;
		m_pSwapChain = nullptr;
	}

	if (m_pID3D12Device != nullptr)
	{
		m_pID3D12Device->Release();
		m_pID3D12Device = nullptr;
	}

	if (m_pIDxgiAdapter != nullptr)
	{
		m_pIDxgiAdapter->Release();
		m_pIDxgiAdapter = nullptr;
	}

	if (m_pIDxgiFactory != nullptr)
	{
		m_pIDxgiFactory->Release();
		m_pIDxgiFactory = nullptr;
	}

#if _DEBUG
	if (m_pIDxgiDebugInterface != nullptr)
	{
		m_pIDxgiDebugInterface->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);

		m_pIDxgiDebugInterface->Release();
		m_pIDxgiDebugInterface = nullptr;
	}

	if (m_hDxgiDebugModule != nullptr)
	{
		FreeLibrary(m_hDxgiDebugModule);
		m_hDxgiDebugModule = nullptr;
	}

	if (m_pID3D12DebugInterface != nullptr)
	{
		m_pID3D12DebugInterface->Release();
		m_pID3D12DebugInterface = nullptr;
	}
#endif

	m_pIWindow = nullptr;
}

bool CGfxContext::EnumerateDxgiAdapters(void)
{
	bool status = true;
	uint32_t uIndex = 0;
	IDXGIAdapter4* pIAdapter = nullptr;
	int32_t AdapterIndex = -1;

	while (status)
	{
		HRESULT result = m_pIDxgiFactory->EnumAdapterByGpuPreference(uIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, __uuidof(IDXGIAdapter4), reinterpret_cast<void**>(&pIAdapter));

		if (result == DXGI_ERROR_NOT_FOUND)
		{
			break;
		}
		else if (result != S_OK)
		{
			Console::Write(L"Error: Could not enumerate adapters\n");
			status = false;
			break;
		}
		else
		{
			status = PrintAdapterProperties(uIndex, pIAdapter);

			if (status)
			{
				if (AdapterIndex == -1)
				{
					// Check if the adapter supports D3D12 - the result is S_FALSE if the function succeeds but ppDevice is NULL
					if (D3D12CreateDevice(pIAdapter, D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr) == S_FALSE)
					{
						AdapterIndex = uIndex;
						m_pIDxgiAdapter = pIAdapter;
					}
				}
			}

			if (pIAdapter != m_pIDxgiAdapter)
			{
				pIAdapter->Release();
			}

			uIndex++;
		}
	}

	if (m_pIDxgiAdapter != nullptr)
	{
		Console::Write(L"Using adapter %u\n", AdapterIndex);
	}
	else
	{
		Console::Write(L"Error: Could not find adapter to use\n");
	}

	return status;
}

bool CGfxContext::PrintAdapterProperties(uint32_t uIndex, IDXGIAdapter4* pIAdapter)
{
	bool status = true;
	DXGI_ADAPTER_DESC3 Desc = { 0 };

	if (pIAdapter->GetDesc3(&Desc) == S_OK)
	{
		Console::Write(L"Adapter %u:\n", uIndex);
		Console::Write(L"\tDescription: %s\n", Desc.Description);
		Console::Write(L"\tVendorId: %X\n", Desc.VendorId);
		Console::Write(L"\tDeviceId: %X\n", Desc.DeviceId);
		Console::Write(L"\tsubSysId: %X\n", Desc.SubSysId);
		Console::Write(L"\tRevision: %X\n", Desc.Revision);
		Console::Write(L"\tDedicatedVideoMemory: %.0f GB\n", ceilf(static_cast<float>(Desc.DedicatedVideoMemory) / static_cast<float>(GB)));
		Console::Write(L"\tDedicatedSystemMemory: %.0f GB\n", ceilf(static_cast<float>(Desc.DedicatedSystemMemory) / static_cast<float>(GB)));
		Console::Write(L"\tSharedSystemMemory: %.0f GB\n", ceilf(static_cast<float>(Desc.SharedSystemMemory) / static_cast<float>(GB)));

		if (Desc.Flags != DXGI_ADAPTER_FLAG3_NONE)
		{
			Console::Write(L"\tFlags:\n");

			if ((Desc.Flags & DXGI_ADAPTER_FLAG3_REMOTE) != 0)
			{
				Console::Write(L"\t\tRemote\n");
			}

			if ((Desc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE) != 0)
			{
				Console::Write(L"\t\tSoftware\n");
			}

			if ((Desc.Flags & DXGI_ADAPTER_FLAG3_ACG_COMPATIBLE) != 0)
			{
				Console::Write(L"\t\tACG_Compatible\n");
			}

			if ((Desc.Flags & DXGI_ADAPTER_FLAG3_SUPPORT_MONITORED_FENCES) != 0)
			{
				Console::Write(L"\t\tSupport_Monitored_Fences\n");
			}

			if ((Desc.Flags & DXGI_ADAPTER_FLAG3_SUPPORT_NON_MONITORED_FENCES) != 0)
			{
				Console::Write(L"\t\tSupport_Non_Monitored_Fences\n");
			}

			if ((Desc.Flags & DXGI_ADAPTER_FLAG3_KEYED_MUTEX_CONFORMANCE) != 0)
			{
				Console::Write(L"\t\tKeyed_Mutex_Conformance\n");
			}
		}

		LPCWCH GraphicsPreemptionGranularity = L"Unknown";
		LPCWCH ComputePreemptionGranularity = L"Unknown";

		switch (Desc.GraphicsPreemptionGranularity)
		{
		case DXGI_GRAPHICS_PREEMPTION_DMA_BUFFER_BOUNDARY:
			GraphicsPreemptionGranularity = L"Dma_Buffer_Boundary";
			break;
		case DXGI_GRAPHICS_PREEMPTION_PRIMITIVE_BOUNDARY:
			GraphicsPreemptionGranularity = L"Primitive_Boundary";
			break;
		case DXGI_GRAPHICS_PREEMPTION_TRIANGLE_BOUNDARY:
			GraphicsPreemptionGranularity = L"Triangle_Boundary";
			break;
		case DXGI_GRAPHICS_PREEMPTION_PIXEL_BOUNDARY:
			GraphicsPreemptionGranularity = L"Pixel_Boundary";
			break;
		case DXGI_GRAPHICS_PREEMPTION_INSTRUCTION_BOUNDARY:
			GraphicsPreemptionGranularity = L"Instruction_Boundary";
			break;
		}

		switch (Desc.ComputePreemptionGranularity)
		{
		case DXGI_COMPUTE_PREEMPTION_DMA_BUFFER_BOUNDARY:
			ComputePreemptionGranularity = L"Dma_Buffer_Boundary";
			break;
		case DXGI_COMPUTE_PREEMPTION_DISPATCH_BOUNDARY:
			ComputePreemptionGranularity = L"Dispatch_Boundary";
			break;
		case DXGI_COMPUTE_PREEMPTION_THREAD_GROUP_BOUNDARY:
			ComputePreemptionGranularity = L"Thread_Group_Boundary";
			break;
		case DXGI_COMPUTE_PREEMPTION_THREAD_BOUNDARY:
			ComputePreemptionGranularity = L"Thread_Boundary";
			break;
		case DXGI_COMPUTE_PREEMPTION_INSTRUCTION_BOUNDARY:
			ComputePreemptionGranularity = L"Instruction_Boundary";
			break;
		}

		Console::Write(L"\tGraphicsPreemptionBoundary: %s\n", GraphicsPreemptionGranularity);
		Console::Write(L"\tComputePreemptionGranularity: %s\n", ComputePreemptionGranularity);
	}
	else
	{
		status = false;
		Console::Write(L"Error: Could not get description for adapter %u\n", uIndex);
	}

	return status;
}

bool CGfxContext::PrintDeviceProperties(void)
{
	bool status = true;

	if (status)
	{
		D3D12_FEATURE_DATA_ARCHITECTURE1 ArchData = {};
		ArchData.NodeIndex = 0;

		if (m_pID3D12Device->CheckFeatureSupport(D3D12_FEATURE_ARCHITECTURE1, &ArchData, sizeof(D3D12_FEATURE_DATA_ARCHITECTURE1)) == S_OK)
		{
			Console::Write(L"DX12 Device Properties:\n");
			Console::Write(L"\tTileBasedRenderer: %s\n", ArchData.TileBasedRenderer ? L"TRUE" : L"FALSE");
			Console::Write(L"\tUMA: %s\n", ArchData.UMA ? L"TRUE" : L"FALSE");
			Console::Write(L"\tCacheCoherentUMA: %s\n", ArchData.CacheCoherentUMA ? L"TRUE" : L"FALSE");
			Console::Write(L"\tIsolatedMMU: %s\n", ArchData.IsolatedMMU ? L"TRUE" : L"FALSE");
		}
		else
		{
			status = false;
			Console::Write(L"Error: Could not get device architecture\n");
		}
	}

	if (status)
	{
		DXGI_QUERY_VIDEO_MEMORY_INFO NonLocalMemoryInfo = {};

		if (m_pIDxgiAdapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL, &NonLocalMemoryInfo) == S_OK)
		{
			Console::Write(L"\tNon-Local Memory Info:\n");
			Console::Write(L"\t\tBudget: %.2f GB\n", static_cast<float>(NonLocalMemoryInfo.Budget) / static_cast<float>(GB));
			Console::Write(L"\t\tCurrentUsage: %.2f GB\n", static_cast<float>(NonLocalMemoryInfo.CurrentUsage) / static_cast<float>(GB));
			Console::Write(L"\t\tAvailableForReservation: %.2f GB\n", static_cast<float>(NonLocalMemoryInfo.AvailableForReservation) / static_cast<float>(GB));
			Console::Write(L"\t\tCurrentReservation: %.2f GB\n", static_cast<float>(NonLocalMemoryInfo.CurrentReservation) / static_cast<float>(GB));
		}
		else
		{
			status = false;
			Console::Write(L"Error: Could not get adapter non-local memory info\n");
		}
	}

	if (status)
	{
		DXGI_QUERY_VIDEO_MEMORY_INFO LocalMemoryInfo = {};

		if (m_pIDxgiAdapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &LocalMemoryInfo) == S_OK)
		{
			Console::Write(L"\tLocal Memory Info:\n");
			Console::Write(L"\t\tBudget: %.2f GB\n", static_cast<float>(LocalMemoryInfo.Budget) / static_cast<float>(GB));
			Console::Write(L"\t\tCurrentUsage: %.2f GB\n", static_cast<float>(LocalMemoryInfo.CurrentUsage) / static_cast<float>(GB));
			Console::Write(L"\t\tAvailableForReservation: %.2f GB\n", static_cast<float>(LocalMemoryInfo.AvailableForReservation) / static_cast<float>(GB));
			Console::Write(L"\t\tCurrentReservation: %.2f GB\n", static_cast<float>(LocalMemoryInfo.CurrentReservation) / static_cast<float>(GB));
		}
		else
		{
			status = false;
			Console::Write(L"Error: Could not get adapter local memory info\n");
		}
	}

	return status;
}

bool CGfxContext::InitializeHeaps(const ContextFactory::Descriptor& rDesc)
{
	bool status = true;

	if (status)
	{
		D3D12_RESOURCE_DESC UploadHeapResourceDesc = { };
		UploadHeapResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		UploadHeapResourceDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		UploadHeapResourceDesc.Width = rDesc.UploadHeapSize;
		UploadHeapResourceDesc.Height = 1;
		UploadHeapResourceDesc.DepthOrArraySize = 1;
		UploadHeapResourceDesc.MipLevels = 1;
		UploadHeapResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		UploadHeapResourceDesc.SampleDesc.Count = 1;
		UploadHeapResourceDesc.SampleDesc.Quality = 0;
		UploadHeapResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		UploadHeapResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		D3D12_RESOURCE_ALLOCATION_INFO UploadHeapAllocationInfo = m_pID3D12Device->GetResourceAllocationInfo(0, 1, &UploadHeapResourceDesc);

		D3D12_HEAP_DESC UploadHeapDesc = { };
		UploadHeapDesc.SizeInBytes = UploadHeapAllocationInfo.SizeInBytes;
		UploadHeapDesc.Properties.Type = D3D12_HEAP_TYPE_UPLOAD;
		UploadHeapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		UploadHeapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		UploadHeapDesc.Properties.CreationNodeMask = 1;
		UploadHeapDesc.Properties.VisibleNodeMask = 1;

		if (m_pID3D12Device->CreateHeap(&UploadHeapDesc, __uuidof(ID3D12Heap), reinterpret_cast<void**>(&m_pID3D12UploadHeap)) != S_OK)
		{
			status = false;
			Console::Write(L"Error: Failed to create upload heap\n");
		}
	}

	if (status)
	{
		D3D12_RESOURCE_DESC PrimaryHeapResourceDesc = { };
		PrimaryHeapResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		PrimaryHeapResourceDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		PrimaryHeapResourceDesc.Width = rDesc.PrimaryHeapSize;
		PrimaryHeapResourceDesc.Height = 1;
		PrimaryHeapResourceDesc.DepthOrArraySize = 1;
		PrimaryHeapResourceDesc.MipLevels = 1;
		PrimaryHeapResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		PrimaryHeapResourceDesc.SampleDesc.Count = 1;
		PrimaryHeapResourceDesc.SampleDesc.Quality = 0;
		PrimaryHeapResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		PrimaryHeapResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		D3D12_RESOURCE_ALLOCATION_INFO PrimaryHeapAllocationInfo = m_pID3D12Device->GetResourceAllocationInfo(0, 1, &PrimaryHeapResourceDesc);

		D3D12_HEAP_DESC PrimaryHeapDesc = { };
		PrimaryHeapDesc.SizeInBytes = PrimaryHeapAllocationInfo.SizeInBytes;
		PrimaryHeapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;
		PrimaryHeapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		PrimaryHeapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		PrimaryHeapDesc.Properties.CreationNodeMask = 1;
		PrimaryHeapDesc.Properties.VisibleNodeMask = 1;

		if (m_pID3D12Device->CreateHeap(&PrimaryHeapDesc, __uuidof(ID3D12Heap), reinterpret_cast<void**>(&m_pID3D12PrimaryHeap)) != S_OK)
		{
			status = false;
			Console::Write(L"Error: Failed to create upload heap\n");
		}
	}

	return status;
}

bool CGfxContext::InitializeDescriptorHeaps(void)
{
	bool status = true;

	if (status)
	{
		D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
		cbvHeapDesc.NodeMask = 0;
		cbvHeapDesc.NumDescriptors = 1;
		cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

		m_pID3D12Device->CreateDescriptorHeap(&cbvHeapDesc, __uuidof(ID3D12DescriptorHeap), reinterpret_cast<void**>(&m_pID3D12ShaderResourceHeap));

		if (m_pID3D12ShaderResourceHeap == nullptr)
		{
			status = false;
			Console::Write(L"Error: Failed to create constant buffer descriptor heap\n");
		}
	}

	return status;
}

bool CGfxContext::InitializeSwapChain(void)
{
	bool status = true;
	IDXGISwapChain4* pIDxgiSwapChain = nullptr;
	ID3D12DescriptorHeap* pIRtvDescriptorHeap = nullptr;

	UINT RtvDescriptorIncrement = 0;
	CSwapChain::Descriptor Descriptor = {};

	if (status)
	{
		D3D12_DESCRIPTOR_HEAP_DESC descHeap = {};
		descHeap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		descHeap.NumDescriptors = CSwapChain::NUM_BUFFERS;
		descHeap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		descHeap.NodeMask = 0;

		if (m_pID3D12Device->CreateDescriptorHeap(&descHeap, __uuidof(ID3D12DescriptorHeap), reinterpret_cast<void**>(&pIRtvDescriptorHeap)) == S_OK)
		{
			RtvDescriptorIncrement = m_pID3D12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

			Descriptor.RtvDescriptorIncrement = RtvDescriptorIncrement;
		}
		else
		{
			status = false;
			Console::Write(L"Error: Failed to create descriptor heap\n");
		}
	}

	if (status)
	{
		WIN_RECT Rect = {};

		if (m_pIWindow->GetRect(WIN_AREA::CLIENT, Rect))
		{
			DXGI_SWAP_CHAIN_DESC1 SwapChainDesc = { };
			SwapChainDesc.Width = Rect.width;
			SwapChainDesc.Height = Rect.height;
			SwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			SwapChainDesc.Stereo = FALSE;
			SwapChainDesc.SampleDesc.Count = 1;
			SwapChainDesc.SampleDesc.Quality = 0;
			SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			SwapChainDesc.BufferCount = CSwapChain::NUM_BUFFERS;
			SwapChainDesc.Scaling = DXGI_SCALING_NONE;
			SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			SwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
			SwapChainDesc.Flags = 0;

			IDXGISwapChain1* pISwapChain1 = nullptr;

			if (m_pIDxgiFactory->CreateSwapChainForHwnd(m_pGraphicsQueue->GetD3D12CommandQueue(), m_pIWindow->GetHandle(), &SwapChainDesc, NULL, NULL, &pISwapChain1) == S_OK)
			{
				pISwapChain1->QueryInterface(__uuidof(IDXGISwapChain4), reinterpret_cast<void**>(&pIDxgiSwapChain));
				pISwapChain1->Release();
			}
			else
			{
				status = false;
				Console::Write(L"Error: Failed to create swap chain\n");
			}
		}
		else
		{
			status = false;
		}
	}

	if (status)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE CpuDescriptor = pIRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

		for (UINT i = 0; status && (i < CSwapChain::NUM_BUFFERS); i++)
		{
			ID3D12Resource* pIRenderBuffer = nullptr;

			if (pIDxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), reinterpret_cast<void**>(&pIRenderBuffer)) != S_OK)
			{
				status = false;
				Console::Write(L"Error: Could not get swap chain buffer %u\n", i);
			}

			
			m_pID3D12Device->CreateRenderTargetView(pIRenderBuffer, NULL, CpuDescriptor);
			Descriptor.RenderBuffers[i].hResource = pIRenderBuffer;
			Descriptor.RenderBuffers[i].CpuDescriptor = CpuDescriptor.ptr;
			CpuDescriptor.ptr += RtvDescriptorIncrement;
		}
	}

	if (status)
	{
		m_pSwapChain = new CSwapChain();

		if (m_pSwapChain != nullptr)
		{
			if (m_pSwapChain->Initialize(pIDxgiSwapChain, pIRtvDescriptorHeap, Descriptor))
			{
				static_cast<CWindow*>(m_pIWindow)->SwapChainNotification(CWindow::SWAPCHAIN_CREATED, m_pSwapChain);
			}
			else
			{
				m_pSwapChain->Uninitialize();
				delete m_pSwapChain;
				m_pSwapChain = nullptr;
			}
		}
	}

	if (!status)
	{
		if (pIDxgiSwapChain != nullptr)
		{
			pIDxgiSwapChain->Release();
			pIDxgiSwapChain = nullptr;
		}

		if (pIRtvDescriptorHeap != nullptr)
		{
			pIRtvDescriptorHeap->Release();
			pIRtvDescriptorHeap = nullptr;
		}
	}

	return status;
}

ICommandQueue* CGfxContext::CreateCommandQueue(COMMAND_QUEUE_TYPE Type)
{
	bool status = true;
	ICommandQueue* pICommandQueue = nullptr;
	D3D12_COMMAND_LIST_TYPE CmdListType = D3D12_COMMAND_LIST_TYPE_DIRECT;
	ID3D12Fence* pID3D12Fence = nullptr;
	ID3D12CommandQueue* pID3D12CommandQueue = nullptr;

	status = EnumTranslator::CommandQueueType_To_CommandListType(Type, CmdListType);

	if (status)
	{
		D3D12_COMMAND_QUEUE_DESC CmdQueueDesc = { };
		CmdQueueDesc.Type = CmdListType;
		CmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		CmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		CmdQueueDesc.NodeMask = 0;

		if (m_pID3D12Device->CreateCommandQueue(&CmdQueueDesc, __uuidof(ID3D12CommandQueue), reinterpret_cast<void**>(&pID3D12CommandQueue)) != S_OK)
		{
			status = false;
			Console::Write(L"Error: Failed to create d3d12 command queue\n");
		}
	}

	if (status)
	{
		if (m_pID3D12Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), reinterpret_cast<void**>(&pID3D12Fence)) != S_OK)
		{
			status = false;
			Console::Write(L"Error: Failed to create d3d12 fence\n");
		}
	}

	if (status)
	{
		pICommandQueue = new CCommandQueue();
		if (pICommandQueue != nullptr)
		{
			if (!static_cast<CCommandQueue*>(pICommandQueue)->Initialize(Type, pID3D12CommandQueue, pID3D12Fence))
			{
				DestroyCommandQueue(pICommandQueue);
				pICommandQueue = nullptr;
			}
		}
	}

	if (!status)
	{
		if (pID3D12CommandQueue != nullptr)
		{
			pID3D12CommandQueue->Release();
			pID3D12CommandQueue = nullptr;
		}

		if (pID3D12Fence != nullptr)
		{
			pID3D12Fence->Release();
			pID3D12Fence = nullptr;
		}
	}

	return pICommandQueue;
}

void CGfxContext::DestroyCommandQueue(ICommandQueue* pICommandQueue)
{
	CCommandQueue* pCommandQueue = static_cast<CCommandQueue*>(pICommandQueue);
	if (pCommandQueue != nullptr)
	{
		pCommandQueue->Uninitialize();
		delete pCommandQueue;
		pCommandQueue = nullptr;
	}
}

IRendererState* CGfxContext::CreateRendererState(const RENDERER_STATE_DESC& rDesc)
{
	bool status = true;

	IRendererState* pIRendererState = nullptr;
	ID3D12PipelineState* pID3D12PipelineState = nullptr;
	ID3D12RootSignature* pID3D12RootSignature = nullptr;
	D3D12_INPUT_ELEMENT_DESC InputElementDescs[MAX_INPUT_ELEMENTS] = {};
	
	if (rDesc.InputLayout.NumInputs <= MAX_INPUT_ELEMENTS)
	{
		for (UINT i = 0; status && (i < rDesc.InputLayout.NumInputs); i++)
		{
			if (status)
			{
				status = EnumTranslator::InputElement_To_SemanticName(rDesc.InputLayout.pInputElements[i].Element, InputElementDescs[i].SemanticName);
			}

			if (status)
			{
				status = EnumTranslator::InputElementFormat_To_DxgiFormat(rDesc.InputLayout.pInputElements[i].ElementFormat, InputElementDescs[i].Format);
			}

			if (status)
			{
				status = EnumTranslator::InputElementType_To_InputSlotClass(rDesc.InputLayout.pInputElements[i].Type, InputElementDescs[i].InputSlotClass);
			}

			if (status)
			{
				InputElementDescs[i].AlignedByteOffset    = rDesc.InputLayout.pInputElements[i].AlignedByteOffset;
				InputElementDescs[i].InputSlot            = rDesc.InputLayout.pInputElements[i].InputSlot;
				InputElementDescs[i].SemanticIndex        = rDesc.InputLayout.pInputElements[i].ElementIndex;
				InputElementDescs[i].InstanceDataStepRate = rDesc.InputLayout.pInputElements[i].InstanceStep;
			}
		}
	}
	else
	{
		status = false;
		Console::Write(L"Error: Too many input elements\n");
	}

	if (status)
	{
		D3D12_ROOT_SIGNATURE_FLAGS Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
		Flags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
		Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
		Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
		Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS;
		Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS;

		D3D12_DESCRIPTOR_RANGE DescriptorTable0 = {};
		DescriptorTable0.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		DescriptorTable0.NumDescriptors = 1;
		DescriptorTable0.BaseShaderRegister = 0;
		DescriptorTable0.RegisterSpace = 0;
		DescriptorTable0.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		D3D12_ROOT_PARAMETER RootParameters[2] = {};

		RootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		RootParameters[0].Descriptor.RegisterSpace = 0;
		RootParameters[0].Descriptor.ShaderRegister = 0;
		RootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

		RootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		RootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
		RootParameters[1].DescriptorTable.pDescriptorRanges = &DescriptorTable0;
		RootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		D3D12_STATIC_SAMPLER_DESC TextureSampler = {};
        TextureSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        TextureSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        TextureSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        TextureSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        TextureSampler.MipLODBias = 0;
        TextureSampler.MaxAnisotropy = 0;
        TextureSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        TextureSampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        TextureSampler.MinLOD = 0.0f;
        TextureSampler.MaxLOD = D3D12_FLOAT32_MAX;
        TextureSampler.ShaderRegister = 0;
        TextureSampler.RegisterSpace = 0;
        TextureSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		D3D12_ROOT_SIGNATURE_DESC Desc = { };
		Desc.NumParameters = 2;
		Desc.pParameters = RootParameters;
		Desc.NumStaticSamplers = 1;
		Desc.pStaticSamplers = &TextureSampler;
		Desc.Flags = Flags;

		ID3DBlob* pSignature = nullptr;
		ID3DBlob* pError = nullptr;

		if (D3D12SerializeRootSignature(&Desc, D3D_ROOT_SIGNATURE_VERSION_1, &pSignature, &pError) == S_OK)
		{
			if (m_pID3D12Device->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), __uuidof(ID3D12RootSignature), reinterpret_cast<void**>(&pID3D12RootSignature)) != S_OK)
			{
				Console::Write(L"Error: Could not create root signature\n");
				status = false;
			}
		}
		else
		{
			Console::Write(L"Error: Could not initialize root signature\n");
			status = false;

			if (pError != nullptr)
			{
				Console::Write(L"Error Info: %s\n", pError->GetBufferPointer());
			}
		}

		if (pSignature != nullptr)
		{
			pSignature->Release();
			pSignature = nullptr;
		}

		if (pError != nullptr)
		{
			pError->Release();
			pError = nullptr;
		}
	}

	if (status)
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC Desc;
		
		ZeroMemory(&Desc, sizeof(Desc));

		// Root Signature
		Desc.pRootSignature     = pID3D12RootSignature;

		// Vertex Shader
		Desc.VS.pShaderBytecode = rDesc.VertexShader.pCode;
		Desc.VS.BytecodeLength  = rDesc.VertexShader.Size;

		// Pixel Shader
		Desc.PS.pShaderBytecode = rDesc.PixelShader.pCode;
		Desc.PS.BytecodeLength  = rDesc.PixelShader.Size;

		// Domain Shader
		Desc.DS.pShaderBytecode = nullptr;
		Desc.DS.BytecodeLength  = 0;

		// Hull Shader
		Desc.HS.pShaderBytecode = nullptr;
		Desc.HS.BytecodeLength  = 0;

		// Geometry Shader
		Desc.GS.pShaderBytecode = nullptr;
		Desc.GS.BytecodeLength  = 0;
	
		// Stream Output
		Desc.StreamOutput.pSODeclaration   = nullptr;
		Desc.StreamOutput.NumEntries       = 0;
		Desc.StreamOutput.pBufferStrides   = nullptr;
		Desc.StreamOutput.NumStrides       = 0;
		Desc.StreamOutput.RasterizedStream = 0;

		// Blend State
		Desc.BlendState.AlphaToCoverageEnable                     = FALSE;
		Desc.BlendState.IndependentBlendEnable                    = FALSE;
		for (unsigned int i = 0; i < 8; i++)
		{
			Desc.BlendState.RenderTarget[i].BlendEnable           = FALSE;
			Desc.BlendState.RenderTarget[i].LogicOpEnable         = FALSE;
			Desc.BlendState.RenderTarget[i].SrcBlend              = D3D12_BLEND_ONE;
			Desc.BlendState.RenderTarget[i].DestBlend             = D3D12_BLEND_ZERO;
			Desc.BlendState.RenderTarget[i].BlendOp               = D3D12_BLEND_OP_ADD;
			Desc.BlendState.RenderTarget[i].SrcBlendAlpha         = D3D12_BLEND_ONE;
			Desc.BlendState.RenderTarget[i].DestBlendAlpha        = D3D12_BLEND_ZERO;
			Desc.BlendState.RenderTarget[i].BlendOpAlpha          = D3D12_BLEND_OP_ADD;
			Desc.BlendState.RenderTarget[i].LogicOp               = D3D12_LOGIC_OP_NOOP;
			Desc.BlendState.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		}

		// Sample Mask
		Desc.SampleMask = UINT_MAX;

		// Rasterizer State
		Desc.RasterizerState.FillMode              = D3D12_FILL_MODE_SOLID;
		Desc.RasterizerState.CullMode              = D3D12_CULL_MODE_BACK;
		Desc.RasterizerState.FrontCounterClockwise = FALSE;
		Desc.RasterizerState.DepthBias             = D3D12_DEFAULT_DEPTH_BIAS;
		Desc.RasterizerState.DepthBiasClamp        = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		Desc.RasterizerState.SlopeScaledDepthBias  = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		Desc.RasterizerState.DepthClipEnable       = rDesc.EnableDepthClipping;
		Desc.RasterizerState.MultisampleEnable     = FALSE;
		Desc.RasterizerState.AntialiasedLineEnable = FALSE;
		Desc.RasterizerState.ForcedSampleCount     = 0;
		Desc.RasterizerState.ConservativeRaster    = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		// Depth Stencil State
		Desc.DepthStencilState.DepthEnable                  = TRUE;
		Desc.DepthStencilState.DepthWriteMask               = D3D12_DEPTH_WRITE_MASK_ZERO;
		Desc.DepthStencilState.DepthFunc                    = D3D12_COMPARISON_FUNC_LESS;
		Desc.DepthStencilState.StencilEnable                = FALSE;
		Desc.DepthStencilState.StencilReadMask              = 0;
		Desc.DepthStencilState.StencilWriteMask             = 0;
		Desc.DepthStencilState.FrontFace.StencilFailOp      = static_cast<D3D12_STENCIL_OP>(0);
		Desc.DepthStencilState.FrontFace.StencilDepthFailOp = static_cast<D3D12_STENCIL_OP>(0);
		Desc.DepthStencilState.FrontFace.StencilPassOp      = static_cast<D3D12_STENCIL_OP>(0);
		Desc.DepthStencilState.FrontFace.StencilFunc        = static_cast<D3D12_COMPARISON_FUNC>(0);
		Desc.DepthStencilState.BackFace.StencilFailOp       = static_cast<D3D12_STENCIL_OP>(0);
		Desc.DepthStencilState.BackFace.StencilDepthFailOp  = static_cast<D3D12_STENCIL_OP>(0);
		Desc.DepthStencilState.BackFace.StencilPassOp       = static_cast<D3D12_STENCIL_OP>(0);
		Desc.DepthStencilState.BackFace.StencilFunc         = static_cast<D3D12_COMPARISON_FUNC>(0);

		// Input Layout
		Desc.InputLayout.pInputElementDescs  = InputElementDescs;
		Desc.InputLayout.NumElements         = rDesc.InputLayout.NumInputs;

		// Index Buffer Strip Cut Value
		Desc.IBStripCutValue                 = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;

		// Primitive Topology Type
		Desc.PrimitiveTopologyType           = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

		// Num Render Targets
		Desc.NumRenderTargets                = 1;

		// Render Target Formats
		Desc.RTVFormats[0]                   = DXGI_FORMAT_R8G8B8A8_UNORM;

		// Depth Stencil Format
		Desc.DSVFormat                       = DXGI_FORMAT_UNKNOWN;

		// Sample Desc
		Desc.SampleDesc.Count                = 1;
		Desc.SampleDesc.Quality              = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;

		// Node Mask
		Desc.NodeMask                        = 0;

		// Cached Pipeline State Object
		Desc.CachedPSO.pCachedBlob           = NULL;
		Desc.CachedPSO.CachedBlobSizeInBytes = 0;
		
		Desc.Flags                           = D3D12_PIPELINE_STATE_FLAG_NONE;

		if (m_pID3D12Device->CreateGraphicsPipelineState(&Desc, __uuidof(ID3D12PipelineState), reinterpret_cast<void**>(&pID3D12PipelineState)) != S_OK)
		{
			status = false;
			Console::Write(L"Error: Failed to create graphics pipeline state object\n");
		}
	}

	if (status)
	{
		pIRendererState = new CRendererState();
		if (pIRendererState != nullptr)
		{
			if (!static_cast<CRendererState*>(pIRendererState)->Initialize(pID3D12RootSignature, pID3D12PipelineState, m_pID3D12ShaderResourceHeap))
			{
				DestroyRendererState(pIRendererState);
				pIRendererState = nullptr;
			}
		}
		else
		{
			pID3D12PipelineState->Release();
			pID3D12RootSignature->Release();
		}
	}

	return pIRendererState;
}

void CGfxContext::DestroyRendererState(IRendererState* pIRendererState)
{
	CRendererState* pRendererState = static_cast<CRendererState*>(pIRendererState);
	if (pRendererState != nullptr)
	{
		pRendererState->Uninitialize();
		delete pRendererState;
		pRendererState = nullptr;
	}
}

IConstantBuffer* CGfxContext::CreateConstantBuffer(const CONSTANT_BUFFER_DESC& rDesc)
{
	bool status = true;
	IConstantBuffer* pIConstantBuffer = nullptr;
	ID3D12Resource* pID3D12ConstantBufferResource = nullptr;
	void* CpuVa = 0;

	if (rDesc.Size % 256 != 0)
	{
		status = false;
		Console::Write(L"Error: constant buffer size invalid - must be aligned to 256 bytes\n");
	}

	// Create the constant buffer allocation
	if (status)
	{
		D3D12_RESOURCE_DESC cbDesc = {};
		cbDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		cbDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		cbDesc.Width = rDesc.Size;
		cbDesc.Height = 1;
		cbDesc.DepthOrArraySize = 1;
		cbDesc.MipLevels = 1;
		cbDesc.Format = DXGI_FORMAT_UNKNOWN;
		cbDesc.SampleDesc.Count = 1;
		cbDesc.SampleDesc.Quality = 0;
		cbDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		cbDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		if (m_pID3D12Device->CreatePlacedResource(m_pID3D12UploadHeap, 0, &cbDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, __uuidof(ID3D12Resource), reinterpret_cast<void**>(&pID3D12ConstantBufferResource)) != S_OK)
		{
			status = false;
		}
	}

	// Create the constant buffer view
	if (status)
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = pID3D12ConstantBufferResource->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = rDesc.Size;

		m_pID3D12Device->CreateConstantBufferView(&cbvDesc, m_pID3D12ShaderResourceHeap->GetCPUDescriptorHandleForHeapStart());
	}

	// Map the constant buffer
	if (status)
	{
		D3D12_RANGE cpuReadRange = {};
		cpuReadRange.Begin = 0;
		cpuReadRange.End = 0;

		if (pID3D12ConstantBufferResource->Map(0, &cpuReadRange, reinterpret_cast<void**>(&CpuVa)) != S_OK)
		{
			status = false;
			Console::Write(L"Error: failed to map constant buffer to cpu\n");
		}
	}

	// Create the constant buffer object
	if (status)
	{
		pIConstantBuffer = new CConstantBuffer();
		if (pIConstantBuffer != nullptr)
		{
			if (!static_cast<CConstantBuffer*>(pIConstantBuffer)->Initialize(pID3D12ConstantBufferResource, CpuVa))
			{
				DestroyConstantBuffer(pIConstantBuffer);
				pIConstantBuffer = nullptr;
			}
		}
	}

	if (!status)
	{
		if (pID3D12ConstantBufferResource != nullptr)
		{
			pID3D12ConstantBufferResource->Unmap(0, NULL);

			pID3D12ConstantBufferResource->Release();
			pID3D12ConstantBufferResource = nullptr;
		}
	}

	return pIConstantBuffer;
}

void CGfxContext::DestroyConstantBuffer(IConstantBuffer* pIConstantBuffer)
{
	CConstantBuffer* pConstantBuffer = reinterpret_cast<CConstantBuffer*>(pIConstantBuffer);
	if (pConstantBuffer != nullptr)
	{
		pConstantBuffer->Uninitialize();
		delete pConstantBuffer;
		pConstantBuffer = nullptr;
	}
}

ICommandBuffer* CGfxContext::CreateCommandBuffer(COMMAND_BUFFER_TYPE Type)
{
	bool status = true;
	D3D12_COMMAND_LIST_TYPE CommandListType = D3D12_COMMAND_LIST_TYPE_DIRECT;
	ICommandBuffer* pICommandBuffer = nullptr;
	ID3D12CommandAllocator* pID3D12CommandAllocator = nullptr;
	ID3D12GraphicsCommandList* pID3D12GraphicsCommandList = nullptr;

	status = EnumTranslator::CommandBufferType_To_CommandListType(Type, CommandListType);

	if (status)
	{
		if (m_pID3D12Device->CreateCommandAllocator(CommandListType, __uuidof(ID3D12CommandAllocator), reinterpret_cast<void**>(&pID3D12CommandAllocator)) != S_OK)
		{
			status = false;
			Console::Write(L"Error: Could not create d3d12 command allocator\n");
		}
	}

	if (status)
	{
		if (m_pID3D12Device->CreateCommandList(0, CommandListType, pID3D12CommandAllocator, NULL, __uuidof(ID3D12GraphicsCommandList), reinterpret_cast<void**>(&pID3D12GraphicsCommandList)) != S_OK)
		{
			status = false;
			Console::Write(L"Error: Failed to create command list\n");
		}
	}

	if (status)
	{
		pICommandBuffer = new CCommandBuffer();
		if (pICommandBuffer != nullptr)
		{
			if (!static_cast<CCommandBuffer*>(pICommandBuffer)->Initialize(Type, pID3D12CommandAllocator, pID3D12GraphicsCommandList))
			{
				DestroyCommandBuffer(pICommandBuffer);
				pICommandBuffer = nullptr;
			}
		}
	}

	if (!status)
	{
		if (pID3D12GraphicsCommandList == nullptr)
		{
			pID3D12GraphicsCommandList->Release();
			pID3D12GraphicsCommandList = nullptr;
		}

		if (pID3D12CommandAllocator == nullptr)
		{
			pID3D12CommandAllocator->Release();
			pID3D12CommandAllocator = nullptr;
		}
	}

	return pICommandBuffer;
}

void CGfxContext::DestroyCommandBuffer(ICommandBuffer* pICommandBuffer)
{
	CCommandBuffer* pCommandBuffer = static_cast<CCommandBuffer*>(pICommandBuffer);
	if (pCommandBuffer != nullptr)
	{
		pCommandBuffer->Uninitialize();
		delete pCommandBuffer;
		pCommandBuffer = nullptr;
	}
}

IVertexBuffer* CGfxContext::CreateVertexBuffer(const void* pVertexData, UINT Size, UINT Stride)
{
	bool status = true;
	IVertexBuffer* pIVertexBuffer = nullptr;
	ID3D12Resource* pID3D12VertexBuffer = nullptr;
	ID3D12Resource* pID3D12VertexDataUploadBuffer = nullptr;
	ID3D12GraphicsCommandList* pICommandList = static_cast<CCommandBuffer*>(m_pICopyCommandBuffer)->GetD3D12Interface();

	if (status)
	{
		D3D12_RESOURCE_DESC VertexBufferDesc = {};
		VertexBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		VertexBufferDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		VertexBufferDesc.Width = Size;
		VertexBufferDesc.Height = 1;
		VertexBufferDesc.DepthOrArraySize = 1;
		VertexBufferDesc.MipLevels = 1;
		VertexBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
		VertexBufferDesc.SampleDesc.Count = 1;
		VertexBufferDesc.SampleDesc.Quality = 0;
		VertexBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		VertexBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		if (m_pID3D12Device->CreatePlacedResource(m_pID3D12PrimaryHeap, 0, &VertexBufferDesc, D3D12_RESOURCE_STATE_COPY_DEST, NULL, __uuidof(ID3D12Resource), reinterpret_cast<void**>(&pID3D12VertexBuffer)) != S_OK)
		{
			status = false;
		}

		if (m_pID3D12Device->CreatePlacedResource(m_pID3D12UploadHeap, 0, &VertexBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, __uuidof(ID3D12Resource), reinterpret_cast<void**>(&pID3D12VertexDataUploadBuffer)) != S_OK)
		{
			status = false;
		}
	}

	if (status)
	{
		byte* VertexBufferCpuVa = nullptr;

		D3D12_RANGE CpuReadRange = {};
		CpuReadRange.Begin = 0;
		CpuReadRange.End = 0;

		if (pID3D12VertexDataUploadBuffer->Map(0, &CpuReadRange, reinterpret_cast<void**>(&VertexBufferCpuVa)) == S_OK)
		{
			CopyMemory(VertexBufferCpuVa, pVertexData, Size);
			pID3D12VertexDataUploadBuffer->Unmap(0, NULL);
		}
		else
		{
			status = false;
			Console::Write(L"Error: Failed to map vertex buffer allocation\n");
		}
	}

	if (status)
	{
		D3D12_RESOURCE_BARRIER Barrier = {};
		Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		Barrier.Transition.pResource = pID3D12VertexBuffer;
		Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON; // Resources decay to the common state when accessed from a copy queue in commmand lists

		pICommandList->CopyResource(pID3D12VertexBuffer, pID3D12VertexDataUploadBuffer);
		pICommandList->ResourceBarrier(1, &Barrier);
		if (pICommandList->Close() != S_OK)
		{
			status = false;
			Console::Write(L"Error: Could not finalize command buffer\n");
		}
	}

	if (status)
	{
		ID3D12CommandList* pICommandLists[] = { pICommandList };

		m_pCopyQueue->GetD3D12CommandQueue()->ExecuteCommandLists(1, pICommandLists);

		status = m_pCopyQueue->Sync();
	}

	if (status)
	{
		pIVertexBuffer = new CVertexBuffer();

		if (pIVertexBuffer != nullptr)
		{
			VERTEX_BUFFER_DESC Descriptor = {};
			Descriptor.GpuVA = pID3D12VertexBuffer->GetGPUVirtualAddress();
			Descriptor.Size = Size;
			Descriptor.Stride = Stride;

			if (!static_cast<CVertexBuffer*>(pIVertexBuffer)->Initialize(pID3D12VertexBuffer, Descriptor))
			{
				status = false;
				DestroyVertexBuffer(pIVertexBuffer);
				pIVertexBuffer = nullptr;
			}
		}
		else
		{
			status = false;
		}
	}

	if (pID3D12VertexDataUploadBuffer != nullptr)
	{
		pID3D12VertexDataUploadBuffer->Release();
		pID3D12VertexDataUploadBuffer = nullptr;
	}

	if (!status && (pID3D12VertexBuffer != nullptr))
	{
		pID3D12VertexBuffer->Release();
		pID3D12VertexBuffer = nullptr;
	}

	return pIVertexBuffer;
}

void CGfxContext::DestroyVertexBuffer(IVertexBuffer* pIVertexBuffer)
{
	CVertexBuffer* pVertexBuffer = static_cast<CVertexBuffer*>(pIVertexBuffer);
	if (pVertexBuffer != nullptr)
	{
		pVertexBuffer->Uninitialize();
		delete pVertexBuffer;
		pVertexBuffer = nullptr;
	}
}

IMesh* CGfxContext::CreateMesh(const MESH_DESC& rDesc)
{
	bool status = true;
	IMesh* pIMesh = nullptr;
	IVertexBuffer* pIVertexBuffer = nullptr;

	if (status)
	{
		pIVertexBuffer = CreateVertexBuffer(rDesc.pVertexData, rDesc.VertexBufferSize, rDesc.VertexStride);

		if (pIVertexBuffer == nullptr)
		{
			status = false;
		}
	}

	if (status)
	{
		pIMesh = new CMesh();
		if (pIMesh != nullptr)
		{
			if (!static_cast<CMesh*>(pIMesh)->Initialize(rDesc.NumVertices, pIVertexBuffer))
			{
				status = false;
				DestroyMesh(pIMesh);
				pIMesh = nullptr;
			}
		}
		else
		{
			status = false;
		}
	}

	if (!status && (pIVertexBuffer != nullptr))
	{
		DestroyVertexBuffer(pIVertexBuffer);
		pIVertexBuffer = nullptr;
	}

	return pIMesh;
}

void CGfxContext::DestroyMesh(IMesh* pIMesh)
{
	CMesh* pMesh = static_cast<CMesh*>(pIMesh);
	if (pMesh != nullptr)
	{
		pMesh->Uninitialize();
		delete pMesh;
		pMesh = nullptr;
	}
}

ITexture* CGfxContext::CreateTexture(const TEXTURE_DESC& rDesc)
{
	bool status = true;
	
	ITexture* pITexture = nullptr;

	ID3D12Resource* pID3D12TextureBuffer = nullptr;
	ID3D12Resource* pID3D12TextureDataUploadBuffer = nullptr;
	ID3D12GraphicsCommandList* pICommandList = static_cast<CCommandBuffer*>(m_pICopyCommandBuffer)->GetD3D12Interface();

	uint64_t TextureBufferSizeInBytes = 0;
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT TextureBufferFootprint = {};

	if (status)
	{
		D3D12_RESOURCE_DESC TextureBufferDesc = {};
		TextureBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		TextureBufferDesc.Alignment = 0;
		TextureBufferDesc.Width = rDesc.Width;
        TextureBufferDesc.Height = rDesc.Height;
		TextureBufferDesc.DepthOrArraySize = 1;
        TextureBufferDesc.MipLevels = 1;
        TextureBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		TextureBufferDesc.SampleDesc.Count = 1;
        TextureBufferDesc.SampleDesc.Quality = 0;
		TextureBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        TextureBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		m_pID3D12Device->GetCopyableFootprints(&TextureBufferDesc, 0, 1, 0, &TextureBufferFootprint, NULL, NULL, &TextureBufferSizeInBytes);

		D3D12_RESOURCE_DESC TextureBufferUploadDesc = {};
		TextureBufferUploadDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		TextureBufferUploadDesc.Alignment = 0;
		TextureBufferUploadDesc.Width = TextureBufferSizeInBytes;
		TextureBufferUploadDesc.Height = 1;
		TextureBufferUploadDesc.DepthOrArraySize = 1;
		TextureBufferUploadDesc.MipLevels = 1;
		TextureBufferUploadDesc.Format = DXGI_FORMAT_UNKNOWN;
		TextureBufferUploadDesc.SampleDesc.Count = 1;
		TextureBufferUploadDesc.SampleDesc.Quality = 0;
		TextureBufferUploadDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		TextureBufferUploadDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		if (m_pID3D12Device->CreatePlacedResource(m_pID3D12PrimaryHeap, 0, &TextureBufferDesc, D3D12_RESOURCE_STATE_COPY_DEST, NULL, __uuidof(ID3D12Resource), reinterpret_cast<void**>(&pID3D12TextureBuffer)) != S_OK)
		{
			status = false;
		}

		if (m_pID3D12Device->CreatePlacedResource(m_pID3D12UploadHeap, 0, &TextureBufferUploadDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, __uuidof(ID3D12Resource), reinterpret_cast<void**>(&pID3D12TextureDataUploadBuffer)) != S_OK)
		{
			status = false;
		}
	}

	if (status)
	{
		byte* TextureBufferCpuVa = nullptr;

		D3D12_RANGE CpuReadRange = {};
		CpuReadRange.Begin = 0;
		CpuReadRange.End = 0;

		if (pID3D12TextureDataUploadBuffer->Map(0, &CpuReadRange, reinterpret_cast<void**>(&TextureBufferCpuVa)) == S_OK)
		{
			CopyMemory(TextureBufferCpuVa, rDesc.pTextureData, TextureBufferSizeInBytes);
			pID3D12TextureDataUploadBuffer->Unmap(0, NULL);
		}
		else
		{
			status = false;
			Console::Write(L"Error: Failed to map texture buffer allocation\n");
		}
	}

	if (status)
	{
		D3D12_RESOURCE_BARRIER Barrier = {};
		Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		Barrier.Transition.pResource = pID3D12TextureBuffer;
		Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE; //D3D12_RESOURCE_STATE_COMMON; // Resources decay to the common state when accessed from a copy queue in commmand lists

		D3D12_TEXTURE_COPY_LOCATION Dst = {};
		Dst.pResource = pID3D12TextureBuffer;
		Dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		Dst.SubresourceIndex = 0;

		D3D12_TEXTURE_COPY_LOCATION Src = {};
		Src.pResource = pID3D12TextureDataUploadBuffer;
		Src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		Src.PlacedFootprint = TextureBufferFootprint;

		pICommandList->CopyTextureRegion(&Dst, 0, 0, 0, &Src, nullptr);
		pICommandList->ResourceBarrier(1, &Barrier);
		if (pICommandList->Close() != S_OK)
		{
			status = false;
			Console::Write(L"Error: Could not finalize command buffer\n");
		}
	}

	if (status)
	{
        D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
		SrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		SrvDesc.Texture2D.MostDetailedMip = 0;
        SrvDesc.Texture2D.MipLevels = 1;
		SrvDesc.Texture2D.PlaneSlice = 0;
		SrvDesc.Texture2D.ResourceMinLODClamp = 0;

        m_pID3D12Device->CreateShaderResourceView(pID3D12TextureBuffer, &SrvDesc, m_pID3D12ShaderResourceHeap->GetCPUDescriptorHandleForHeapStart());
	}

	if (status)
	{
		pITexture = new CTexture();

		if (pITexture != nullptr)
		{
			if (!static_cast<CTexture*>(pITexture)->Initialize(pID3D12TextureBuffer))
			{
				status = false;
				DestroyTexture(pITexture);
				pITexture = nullptr;
			}
		}
		else
		{
			status = false;
		}
	}

	if (pID3D12TextureDataUploadBuffer != nullptr)
	{
		pID3D12TextureDataUploadBuffer->Release();
		pID3D12TextureDataUploadBuffer = nullptr;
	}

	if (!status && (pID3D12TextureBuffer != nullptr))
	{
		pID3D12TextureBuffer->Release();
		pID3D12TextureBuffer = nullptr;
	}

	return pITexture;
}

void CGfxContext::DestroyTexture(ITexture* pITexture)
{
	CTexture* pTexture = static_cast<CTexture*>(pITexture);
	if (pTexture != nullptr)
	{
		pTexture->Uninitialize();
		delete pTexture;
		pTexture = nullptr;
	}
}

bool CGfxContext::SubmitCommandBuffer(ICommandBuffer* pICommandBuffer)
{
	bool status = true;

	if (pICommandBuffer == nullptr)
	{
		status = false;
	}

	if (status)
	{
		CCommandBuffer* pCommandBuffer = static_cast<CCommandBuffer*>(pICommandBuffer);
		
		switch (pCommandBuffer->GetType())
		{
			case COMMAND_BUFFER_TYPE_GRAPHICS:
			{
				m_pGraphicsQueue->SubmitCommandBuffer(pICommandBuffer);
				break;
			}
			default:
			{
				status = false;
				Console::Write(L"Error: Trying to submit unsupported command buffer\n");
				break;
			}
		}
	}

	return status;
}

bool CGfxContext::SyncQueue(COMMAND_QUEUE_TYPE Type)
{
	bool status = true;

	switch (Type)
	{
		case COMMAND_QUEUE_TYPE_GRAPHICS:
		{
			status = m_pGraphicsQueue->Sync();
			break;
		}
		case COMMAND_QUEUE_TYPE_COPY:
		{
			status = m_pCopyQueue->Sync();
			break;
		}
		default:
		{
			status = false;
			Console::Write(L"Error: Trying to sync unsupported command queue\n");
			break;
		}
	}

	return status;
}
