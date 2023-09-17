
#include <Cg.hpp>
#include <CgGfx.hpp>
#include <CgSystem.hpp>

#include <vector>

const wchar_t* WINDOW_CLASS = L"HelloTexture";
const wchar_t* WINDOW_TITLE = L"Hello Texture";
const uint32_t WINDOW_HEIGHT  = 512;
const uint32_t WINDOW_WIDTH   = 512;

class HelloTexture
{
private:
	IWindow*        m_pIWindow;
	IGfxContext*    m_pIGfxContext;
	IRendererState* m_pIRendererState;
	IMesh*          m_pIMesh;
	ICommandBuffer* m_pIGraphicsCommandBuffer;
	ITexture*       m_pITexture;

	const float     m_ClearColor[4] = { 0, 0, 0, 0 };

public:
	HelloTexture()
	{
		m_pIWindow = nullptr;
		m_pIGfxContext = nullptr;
		m_pIRendererState = nullptr;
		m_pIMesh = nullptr;
		m_pIGraphicsCommandBuffer = nullptr;
		m_pITexture = nullptr;
	}

	bool Initialize(void)
	{
		bool status = true;
		std::vector<wchar_t> module_directory(1024);

		if (status)
		{
			m_pIWindow = WindowFactory::CreateInstance(WINDOW_CLASS, WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
			if (m_pIWindow == nullptr)
			{
				status = false;
				Console::Write(L"Error: Could not initialize window\n");
			}
		}

		if (status)
		{
			ContextFactory::Descriptor GfxContextDesc = {};
			GfxContextDesc.UploadHeapSize = static_cast<uint64_t>(64U * MB);
			GfxContextDesc.PrimaryHeapSize = static_cast<uint64_t>(64U * MB);

			m_pIGfxContext = ContextFactory::CreateInstance(m_pIWindow, GfxContextDesc);
			if (m_pIGfxContext == nullptr)
			{
				status = false;
				Console::Write(L"Error: Could not initialize graphics context\n");
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
				{ INPUT_ELEMENT_UV,       0, INPUT_ELEMENT_FORMAT_XY_32F,  0, sizeof(float) * 3, INPUT_ELEMENT_TYPE_PER_VERTEX, 0 },
			};

			ReadShaderBytecode(FILE_PATH(module_directory.data(), L"/VertexShader.cso"), Desc.VertexShader);
			ReadShaderBytecode(FILE_PATH(module_directory.data(), L"/PixelShader.cso"),  Desc.PixelShader);

			Desc.InputLayout.pInputElements = InputElements;
			Desc.InputLayout.NumInputs = sizeof(InputElements) / sizeof(INPUT_ELEMENT_DESC);

			m_pIRendererState = m_pIGfxContext->CreateRendererState(Desc);
			if (m_pIRendererState == nullptr)
			{
				status = false;
				Console::Write(L"Error: Could not initialize renderer\n");
			}

			ReleaseShaderBytecode(Desc.VertexShader);
			ReleaseShaderBytecode(Desc.PixelShader);
		}

		if (status)
		{
			m_pIGraphicsCommandBuffer = m_pIGfxContext->CreateCommandBuffer(COMMAND_BUFFER_TYPE_GRAPHICS);

			if (m_pIGraphicsCommandBuffer == nullptr)
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
				float uv[2];
			};

			static const Vertex Vertices[] =
			{
				{
					{  1.0f, -1.0f,  0.0f }, // right
					{  0.0f,  0.0f }
				},
				{
					{ -1.0f, -1.0f,  0.0f }, // left
					{  1.0f,  0.0f }
				},
				{
					{  0.0f,  1.0f,  0.0f }, // top
					{  0.0f,  1.0f }
				}
			};

			MESH_DESC MeshDesc = {};
			MeshDesc.pVertexData = reinterpret_cast<const void*>(Vertices);
			MeshDesc.VertexBufferSize = sizeof(Vertices);
			MeshDesc.VertexStride = sizeof(Vertex);
			MeshDesc.NumVertices = sizeof(Vertices) / sizeof(Vertex);

			m_pIMesh = m_pIGfxContext->CreateMesh(MeshDesc);

			if (m_pIMesh == nullptr)
			{
				status = false;
				Console::Write(L"Error: could not create mesh\n");
			}
		}

		if (status)
		{
			byte color = 0;

			byte TextureData[256][256][4];
			for (uint32_t x = 0; x < 256; x++)
			{
				for (uint32_t y = 0; y < 256; y++)
				{
					TextureData[x][y][0] = color;
					TextureData[x][y][1] =   0;
					TextureData[x][y][2] =   0;
					TextureData[x][y][3] = 255;

					color = (color == 255) ? 0 : color + 1;
				}
			}

			TEXTURE_DESC TextureDesc = {};
			TextureDesc.Width = 256;
			TextureDesc.Height = 256;
			TextureDesc.pTextureData = &TextureData[0][0][0];

			m_pITexture = m_pIGfxContext->CreateTexture(TextureDesc);

			if (m_pITexture == nullptr)
			{
				status = false;
				Console::Write(L"Error: could not create texture\n");
			}
		}

		if (!status)
		{
			Uninitialize();
		}

		return status;
	}

	void Uninitialize(void)
	{
		if (m_pITexture != nullptr)
		{
			m_pIGfxContext->DestroyTexture(m_pITexture);
			m_pITexture = nullptr;
		}

		if (m_pIMesh != nullptr)
		{
			m_pIGfxContext->DestroyMesh(m_pIMesh);
			m_pIMesh = nullptr;
		}

		if (m_pIGraphicsCommandBuffer != nullptr)
		{
			m_pIGfxContext->DestroyCommandBuffer(m_pIGraphicsCommandBuffer);
			m_pIGraphicsCommandBuffer = nullptr;
		}

		if (m_pIRendererState != nullptr)
		{
			m_pIGfxContext->DestroyRendererState(m_pIRendererState);
			m_pIRendererState = nullptr;
		}

		if (m_pIGfxContext != nullptr)
		{
			ContextFactory::DestroyInstance(m_pIGfxContext);
			m_pIGfxContext = nullptr;
		}

		if (m_pIWindow != nullptr)
		{
			WindowFactory::DestroyInstance(m_pIWindow);
			m_pIWindow = nullptr;
		}
	}

private:
	bool Render(void)
	{
		bool status = true;

		status = m_pIGraphicsCommandBuffer->Reset(m_pIRendererState);

		if (status)
		{
			RenderBuffer CurrentBuffer = m_pIWindow->GetCurrentRenderBuffer();

			m_pIGraphicsCommandBuffer->SetViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, MIN_DEPTH, MAX_DEPTH);
			m_pIGraphicsCommandBuffer->SetRenderTarget(CurrentBuffer);
			m_pIGraphicsCommandBuffer->ClearRenderBuffer(CurrentBuffer, m_ClearColor);
			
			m_pIGraphicsCommandBuffer->SetVertexBuffers(1, m_pIMesh->GetVertexBuffer());
			m_pIGraphicsCommandBuffer->Draw(m_pIMesh->GetVertexCount());
			m_pIGraphicsCommandBuffer->Present(CurrentBuffer);

			status = m_pIGraphicsCommandBuffer->Finalize();
		}

		if (status)
		{
			m_pIGfxContext->SubmitCommandBuffer(m_pIGraphicsCommandBuffer);
		}

		if (status)
		{
			status = m_pIWindow->Present();
		}

		if (status)
		{
			status = m_pIGfxContext->SyncQueue(COMMAND_QUEUE_TYPE_GRAPHICS);
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

	HelloTexture hello_texture;

	if (!hello_texture.Initialize())
	{
		status = STATUS::UNSUCCESSFUL;
	}

	if (status == STATUS::SUCCESS)
	{
		if (!hello_texture.Run())
		{
			status = STATUS::UNSUCCESSFUL;
		}
	}

	hello_texture.Uninitialize();

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
