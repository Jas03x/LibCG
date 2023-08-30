#ifndef CG_GFX__HPP
#define CG_GFX__HPP

#include <CgDef.hpp>

class IWindow;

const float MIN_DEPTH = 0.0f;
const float MAX_DEPTH = 1.0f;

// Shared structure between system and gfx
#ifndef CG_RENDER_BUFFER
#define CG_RENDER_BUFFER
struct RenderBuffer
{
	HANDLE   hResource;
	uint64_t CpuDescriptor;
};
#endif // CG_RENDER_BUFFER

// IRendererState
struct SHADER_BYTECODE
{
	byte*    pCode;
	uint32_t Size;
};

enum INPUT_ELEMENT
{
	INPUT_ELEMENT_INVALID = 0,
	INPUT_ELEMENT_POSITION = 1,
	INPUT_ELEMENT_COLOR = 2
};

enum INPUT_ELEMENT_FORMAT
{
	INPUT_ELEMENT_FORMAT_INVALID = 0,
		INPUT_ELEMENT_FORMAT_XYZ_8F = 1, INPUT_ELEMENT_FORMAT_RGB_8F = 1,
		INPUT_ELEMENT_FORMAT_XYZ_16F = 2, INPUT_ELEMENT_FORMAT_RGB_16F = 2,
		INPUT_ELEMENT_FORMAT_XYZ_32F = 3, INPUT_ELEMENT_FORMAT_RGB_32F = 3,
		INPUT_ELEMENT_FORMAT_XYZW_8F = 4, INPUT_ELEMENT_FORMAT_RGBA_8F = 4,
		INPUT_ELEMENT_FORMAT_XYZW_16F = 5, INPUT_ELEMENT_FORMAT_RGBA_16F = 5,
		INPUT_ELEMENT_FORMAT_XYZW_32F = 6, INPUT_ELEMENT_FORMAT_RGBA_32F = 6,
};

enum INPUT_ELEMENT_TYPE
{
	INPUT_ELEMENT_TYPE_INVALID = 0,
	INPUT_ELEMENT_TYPE_PER_VERTEX = 1,
	INPUT_ELEMENT_TYPE_PER_INSTANCE = 2
};

struct INPUT_ELEMENT_DESC
{
	INPUT_ELEMENT        Element;
	uint32_t             ElementIndex;
	INPUT_ELEMENT_FORMAT ElementFormat;
	uint32_t             InputSlot;
	uint32_t             AlignedByteOffset;
	INPUT_ELEMENT_TYPE   Type;
	uint32_t             InstanceStep;
};

struct INPUT_BUFFER_LAYOUT
{
	INPUT_ELEMENT_DESC* pInputElements;
	uint32_t            NumInputs;
};

struct RENDERER_STATE_DESC
{
	bool                EnableDepthClipping;

	SHADER_BYTECODE     VertexShader;
	SHADER_BYTECODE     PixelShader;
	INPUT_BUFFER_LAYOUT InputLayout;
};

class __declspec(novtable) IRendererState
{
public:
};

bool ReadShaderBytecode(const FILE_PATH& Path, SHADER_BYTECODE& rDesc);
void ReleaseShaderBytecode(SHADER_BYTECODE& rDesc);

// IConstantBuffer
struct CONSTANT_BUFFER_DESC
{
	uint32_t Size;
};

class __declspec(novtable) IConstantBuffer
{
public:
	virtual void* GetCpuVA(void) = 0;
};

// IVertexBuffer
struct VERTEX_BUFFER_DESC
{
	uint64_t GpuVA;
	uint32_t Size;
	uint32_t Stride;
};

class __declspec(novtable) IVertexBuffer
{
public:
	virtual const VERTEX_BUFFER_DESC& GetDesc(void) const = 0;
};

// IMesh
struct MESH_DESC
{
	const void* pVertexData;
	uint32_t    VertexBufferSize;
	uint32_t    VertexStride;
	uint32_t    NumVertices;
};

class __declspec(novtable) IMesh
{
public:
	virtual uint32_t       GetVertexCount(void) = 0;
	virtual IVertexBuffer* GetVertexBuffer(void) = 0;
};

// ITexture
struct TEXTURE_DESC
{
	uint32_t Width;
	uint32_t Height;
	byte*    pTextureData;
	uint32_t TextureDataSize;
};

class __declspec(novtable) ITexture
{
public:
};

// ICommandBuffer
enum COMMAND_BUFFER_TYPE
{
	COMMAND_BUFFER_TYPE_INVALID = 0,
	COMMAND_BUFFER_TYPE_DIRECT = 1,
	COMMAND_BUFFER_TYPE_GRAPHICS = 1,
	COMMAND_BUFFER_TYPE_COMPUTE = 2,
	COMMAND_BUFFER_TYPE_COPY = 3
};

class __declspec(novtable) ICommandBuffer
{
public:
	virtual void ClearRenderBuffer(const RenderBuffer& rBuffer, const float RGBA[]) = 0;
	virtual void Present(const RenderBuffer& rBuffer) = 0;
	virtual void SetVertexBuffers(uint32_t NumBuffers, const IVertexBuffer* pIVertexBuffers) = 0;
	virtual void SetConstantBuffer(uint32_t Index, IConstantBuffer* pIConstantBuffer) = 0;
	virtual void Draw(uint32_t NumVertices) = 0;
	virtual void SetViewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h, float min_depth, float max_depth) = 0;
	virtual void ProgramPipeline(IRendererState* pIRendererState) = 0;
	virtual void SetRenderTarget(const RenderBuffer& rBuffer) = 0;

	virtual bool Finalize(void) = 0;
	virtual bool Reset(void) = 0;
};

// class IQueue
enum COMMAND_QUEUE_TYPE
{
	COMMAND_QUEUE_TYPE_INVALID = 0,
	COMMAND_QUEUE_TYPE_DIRECT = 1,
	COMMAND_QUEUE_TYPE_GRAPHICS = 1,
	COMMAND_QUEUE_TYPE_COMPUTE = 2,
	COMMAND_QUEUE_TYPE_COPY = 3
};

class __declspec(novtable) ICommandQueue
{
public:
};

// IGfxContext
class __declspec(novtable) IGfxContext
{
public:
	virtual ICommandBuffer*  CreateCommandBuffer(COMMAND_BUFFER_TYPE Type) = 0;
	virtual void             DestroyCommandBuffer(ICommandBuffer* pICommandBuffer) = 0;

	virtual IRendererState*  CreateRendererState(const RENDERER_STATE_DESC& rDesc) = 0;
	virtual void             DestroyRendererState(IRendererState* pIRendererState) = 0;

	virtual IConstantBuffer* CreateConstantBuffer(const CONSTANT_BUFFER_DESC& rDesc) = 0;
	virtual void             DestroyConstantBuffer(IConstantBuffer* pIConstantBuffer) = 0;

	virtual IVertexBuffer*   CreateVertexBuffer(const void* pVertexData, uint32_t Size, uint32_t Stride) = 0;
	virtual void             DestroyVertexBuffer(IVertexBuffer* pIVertexBuffer) = 0;

	virtual IMesh*           CreateMesh(const MESH_DESC& rDesc) = 0;
	virtual void             DestroyMesh(IMesh* pIMesh) = 0;

	virtual ITexture*        CreateTexture(const TEXTURE_DESC& rDesc) = 0;
	virtual void             DestroyTexture(ITexture* pITexture) = 0;

	virtual bool             SubmitCommandBuffer(ICommandBuffer* pICommandBuffer) = 0;
	virtual bool             SyncQueue(COMMAND_QUEUE_TYPE Type) = 0;
};

// Context Factory
class __declspec(novtable) ContextFactory
{
public:
	struct Descriptor
	{
		uint64_t UploadHeapSize;
		uint64_t PrimaryHeapSize;
	};

	static IGfxContext* CreateInstance(IWindow* pIWindow, const Descriptor& rDesc);
	static void         DestroyInstance(IGfxContext* pIContext);
};

#endif // CG_GFX__HPP
