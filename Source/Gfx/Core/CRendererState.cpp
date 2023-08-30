#include "CRendererState.hpp"

#include <d3d12.h>

#include "Cg.hpp"

bool ReadShaderBytecode(const FILE_PATH& Path, SHADER_BYTECODE& rDesc)
{
	bool status = true;

	File* pFile = File::Open(Path);
	if (pFile == nullptr)
	{
		status = false;
		Console::Write(L"Error: Failed to open shader file %s for reading\n", Path.FileName);
	}

	if (status)
	{
		if (!pFile->Read(&rDesc.pCode, &rDesc.Size) )
		{
			status = false;
			Console::Write(L"Error: Failed to read shader file %s\n", Path.FileName);
		}
	}

	if (pFile != nullptr)
	{
		File::Close(pFile);
		pFile = nullptr;
	}

	return status;
}

void ReleaseShaderBytecode(SHADER_BYTECODE& rDesc)
{
	if (rDesc.pCode != nullptr)
	{
		Memory::Release(rDesc.pCode);
		rDesc.pCode = nullptr;
	}

	rDesc.Size = 0;
}

CRendererState::CRendererState(void)
{
	m_pID3D12PipelineState = nullptr;
	m_pID3D12RootSignature = nullptr;
	m_pShaderResourceHeap  = nullptr;
}

CRendererState::~CRendererState(void)
{
}

bool CRendererState::Initialize(ID3D12RootSignature* pIRootSignature, ID3D12PipelineState* pIPipelineState, ID3D12DescriptorHeap* pShaderResourceHeap)
{
	bool status = true;

	if ((pIRootSignature != nullptr) && (pIPipelineState != nullptr) && (pShaderResourceHeap != nullptr))
	{
		m_pID3D12PipelineState = pIPipelineState;
		m_pID3D12RootSignature = pIRootSignature;

		m_pShaderResourceHeap = pShaderResourceHeap;
		pShaderResourceHeap->AddRef();
	}
	else
	{
		status = false;
		Console::Write(L"Error: Invalid pointer\n");
	}

	return status;
}

void CRendererState::Uninitialize(void)
{
	if (m_pShaderResourceHeap != nullptr)
	{
		m_pShaderResourceHeap->Release();
		m_pShaderResourceHeap = nullptr;
	}

	if (m_pID3D12PipelineState != nullptr)
	{
		m_pID3D12PipelineState->Release();
		m_pID3D12PipelineState = nullptr;
	}

	if (m_pID3D12RootSignature != nullptr)
	{
		m_pID3D12RootSignature->Release();
		m_pID3D12RootSignature = nullptr;
	}
}

ID3D12PipelineState* CRendererState::GetD3D12PipelineState(void)
{
	return m_pID3D12PipelineState;
}

ID3D12RootSignature* CRendererState::GetD3D12RootSignature(void)
{
	return m_pID3D12RootSignature;
}

ID3D12DescriptorHeap* CRendererState::GetShaderResourceHeap(void)
{
	return m_pShaderResourceHeap;
}
