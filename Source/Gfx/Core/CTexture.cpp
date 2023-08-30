#include "CTexture.hpp"

#include <d3d12.h>

CTexture::CTexture(void)
{
    m_pID3D12TextureBuffer = nullptr;
}

CTexture::~CTexture(void)
{

}

bool CTexture::Initialize(ID3D12Resource* pID3D12TextureBuffer)
{
    bool status = true;
    
    if (pID3D12TextureBuffer != nullptr)
    {
        m_pID3D12TextureBuffer = pID3D12TextureBuffer;
    }
    else
    {
        status = false;
    }

    return status;
}

void CTexture::Uninitialize(void)
{
    if (m_pID3D12TextureBuffer != nullptr)
    {
        m_pID3D12TextureBuffer->Release();
        m_pID3D12TextureBuffer = nullptr;
    }
}
