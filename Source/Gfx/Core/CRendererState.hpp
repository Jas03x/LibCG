#ifndef CG_CRENDERER_STATE_HPP
#define CG_CRENDERER_STATE_HPP

#include "Cg.hpp"
#include "CgGfx.hpp"

struct ID3D12PipelineState;
struct ID3D12RootSignature;
struct ID3D12DescriptorHeap;

class CRendererState : public IRendererState
{
private:
	ID3D12PipelineState*  m_pID3D12PipelineState;
	ID3D12RootSignature*  m_pID3D12RootSignature;

	ID3D12DescriptorHeap* m_pShaderResourceHeap;

public:
	CRendererState(void);
	~CRendererState(void);

	bool Initialize(ID3D12RootSignature* pIRootSignature, ID3D12PipelineState* pIPipelineState, ID3D12DescriptorHeap* pShaderResourceHeap);
	void Uninitialize(void);

	ID3D12PipelineState*  GetD3D12PipelineState(void);
	ID3D12RootSignature*  GetD3D12RootSignature(void);
	ID3D12DescriptorHeap* GetShaderResourceHeap(void);
};

bool ReadShaderBytecode(const FILE_PATH& Path, SHADER_BYTECODE& rDesc);
void ReleaseShaderBytecode(SHADER_BYTECODE& rDesc);

#endif // CG_CRENDERER_STATE_HPP
