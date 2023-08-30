
#include <Cg.hpp>
#include <CgGfx.hpp>
#include <CgSystem.hpp>

#include <vector>

const wchar_t* WINDOW_CLASS = L"HelloTriangle";
const wchar_t* WINDOW_TITLE = L"Hello Triangle";
const uint32_t WINDOW_HEIGHT  = 512;
const uint32_t WINDOW_WIDTH   = 512;

class HelloTriangle
{
private:
	IWindow*        m_pIWindow;
	IGfxDevice*     m_pIGfxDevice;
	IRendererState* m_pIRendererState;
	IMesh*          m_pIMesh;
	ICommandBuffer* m_pIGraphicsCommandBuffer;

	const float     m_ClearColor[4] = { 0, 0, 0, 0 };

public:
	HelloTriangle()
	{
		m_pIWindow = NULL;
		m_pIGfxDevice = NULL;
		m_pIRendererState = NULL;
		m_pIMesh = NULL;
		m_pIGraphicsCommandBuffer = NULL;
	}

	bool Initialize(void)
	{
		bool status = true;
		std::vector<wchar_t> module_directory(1024);

		if (status)
		{
			m_pIWindow = WindowFactory::CreateInstance(WINDOW_CLASS, WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
			if (m_pIWindow == NULL)
			{
				status = false;
				Console::Write(L"Error: Could not initialize window\n");
			}
		}

		if (status)
		{
			DeviceFactory::Descriptor GfxDeviceDesc = {};
			GfxDeviceDesc.UploadHeapSize = static_cast<uint64_t>(64U * MB);
			GfxDeviceDesc.PrimaryHeapSize = static_cast<uint64_t>(64U * MB);

			m_pIGfxDevice = DeviceFactory::CreateInstance(m_pIWindow, GfxDeviceDesc);
			if (m_pIGfxDevice == NULL)
			{
				status = false;
				Console::Write(L"Error: Could not initialize graphics device\n");
			}
		}
		
		if (status)
		{
			status = System::GetModuleDirectory(module_directory.data(), static_cast<uint32_t>(module_directory.size()));
		}

		if (status)
		{
			RENDERER_STATE_DESC Desc = {};

			INPUT_ELEMENT_DESC InputElements[] =
			{
				{ INPUT_ELEMENT_POSITION, 0, INPUT_ELEMENT_FORMAT_XYZ_32F, 0, sizeof(float) * 0, INPUT_ELEMENT_TYPE_PER_VERTEX, 0 },
				{ INPUT_ELEMENT_COLOR,    0, INPUT_ELEMENT_FORMAT_RGB_32F, 0, sizeof(float) * 3, INPUT_ELEMENT_TYPE_PER_VERTEX, 0 },
			};

			ReadShaderBytecode(FILE_PATH(module_directory.data(), L"/VertexShader.cso"), Desc.VertexShader);
			ReadShaderBytecode(FILE_PATH(module_directory.data(), L"/PixelShader.cso"),  Desc.PixelShader);

			Desc.InputLayout.pInputElements = InputElements;
			Desc.InputLayout.NumInputs = sizeof(InputElements) / sizeof(INPUT_ELEMENT_DESC);

			m_pIRendererState = m_pIGfxDevice->CreateRendererState(Desc);
			if (m_pIRendererState == NULL)
			{
				status = false;
				Console::Write(L"Error: Could not initialize renderer\n");
			}

			ReleaseShaderBytecode(Desc.VertexShader);
			ReleaseShaderBytecode(Desc.PixelShader);
		}

		if (status)
		{
			m_pIGraphicsCommandBuffer = m_pIGfxDevice->CreateCommandBuffer(COMMAND_BUFFER_TYPE_GRAPHICS);

			if (m_pIGraphicsCommandBuffer == NULL)
			{
				status = false;
				Console::Write(L"Error: could not create graphics command buffer\n");
			}
		}

		if (status)
		{
			struct Vertex
			{
				float vertex[3];
				float colour[3];
			};

			static const Vertex Vertices[] =
			{
				{
					{  1.0f, -1.0f,  0.0f }, // right
					{  0.0f,  1.0f,  0.0f }  // green
				},
				{
					{ -1.0f, -1.0f,  0.0f }, // left
					{  0.0f,  0.0f,  1.0f }  // blue
				},
				{
					{  0.0f,  1.0f,  0.0f }, // top
					{  1.0f,  0.0f,  0.0f }  // red
				}
			};

			MESH_DESC MeshDesc = {};
			MeshDesc.pVertexData = reinterpret_cast<const void*>(Vertices);
			MeshDesc.VertexBufferSize = sizeof(Vertices);
			MeshDesc.VertexStride = sizeof(Vertex);
			MeshDesc.NumVertices = sizeof(Vertices) / sizeof(Vertex);

			m_pIMesh = m_pIGfxDevice->CreateMesh(MeshDesc);

			if (m_pIMesh == NULL)
			{
				status = false;
				Console::Write(L"Error: could not create mesh\n");
			}
		}

		if (status == false)
		{
			Uninitialize();
		}

		return status;
	}

	void Uninitialize(void)
	{
		if (m_pIMesh != NULL)
		{
			m_pIGfxDevice->DestroyMesh(m_pIMesh);
			m_pIMesh = NULL;
		}

		if (m_pIGraphicsCommandBuffer != NULL)
		{
			m_pIGfxDevice->DestroyCommandBuffer(m_pIGraphicsCommandBuffer);
			m_pIGraphicsCommandBuffer = NULL;
		}

		if (m_pIRendererState != NULL)
		{
			m_pIGfxDevice->DestroyRendererState(m_pIRendererState);
			m_pIRendererState = NULL;
		}

		if (m_pIGfxDevice != NULL)
		{
			DeviceFactory::DestroyInstance(m_pIGfxDevice);
			m_pIGfxDevice = NULL;
		}

		if (m_pIWindow != NULL)
		{
			WindowFactory::DestroyInstance(m_pIWindow);
			m_pIWindow = NULL;
		}
	}

private:
	bool Render(void)
	{
		bool status = true;

		status = m_pIGraphicsCommandBuffer->Reset();

		if (status)
		{
			RenderBuffer CurrentBuffer = m_pIWindow->GetCurrentRenderBuffer();

			m_pIGraphicsCommandBuffer->SetViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, MIN_DEPTH, MAX_DEPTH);
			m_pIGraphicsCommandBuffer->SetRenderTarget(CurrentBuffer);
			m_pIGraphicsCommandBuffer->ClearRenderBuffer(CurrentBuffer, m_ClearColor);
			
			m_pIGraphicsCommandBuffer->ProgramPipeline(m_pIRendererState);
			m_pIGraphicsCommandBuffer->SetVertexBuffers(1, m_pIMesh->GetVertexBuffer());
			m_pIGraphicsCommandBuffer->Draw(m_pIMesh->GetVertexCount());
			m_pIGraphicsCommandBuffer->Present(CurrentBuffer);

			status = m_pIGraphicsCommandBuffer->Finalize();
		}

		if (status)
		{
			m_pIGfxDevice->SubmitCommandBuffer(m_pIGraphicsCommandBuffer);
		}

		if (status)
		{
			status = m_pIWindow->Present();
		}

		if (status)
		{
			status = m_pIGfxDevice->SyncQueue(COMMAND_QUEUE_TYPE_GRAPHICS);
		}

		return status;
	}

public:
	bool Run()
	{
		bool status = true;
		WIN_EVENT WinEvent = {};

		while (status && (m_pIWindow->Open()))
		{
			if (m_pIWindow->GetEvent(WinEvent))
			{
				if (WinEvent.msg == WIN_MSG::QUIT)
				{
					break;
				}
			}
			else
			{
				status = Render();
			}
		}

		return status;
	}
};

int32_t CgMain(int32_t argc, const wchar_t* argv[])
{
	int32_t status = STATUS::SUCCESS;

	Console::Write(L"Hello Triangle!\n");

	HelloTriangle hello_triangle;

	if (!hello_triangle.Initialize())
	{
		status = STATUS::UNSUCCESSFUL;
	}

	if (status == STATUS::SUCCESS)
	{
		if (!hello_triangle.Run())
		{
			status = STATUS::UNSUCCESSFUL;
		}
	}

	hello_triangle.Uninitialize();

	if (status == STATUS::SUCCESS)
	{
		Console::Write(L"Goodbye Triangle!\n");
	}
	else
	{
		Console::Write(L"Exit with error 0x%X\n", status);
	}
	
	return status;
}
