#include "CTexture.hpp"

#include <d3d12.h>

#include "Cg.hpp"
#include "CAllocation.hpp"

CTexture::CTexture(void)
{
    m_pAllocation = nullptr;
}

CTexture::~CTexture(void)
{
    CgAssert(m_pAllocation == nullptr, L"Texture allocation not de-allocated\n");
}

bool CTexture::Initialize(CAllocation* pAllocation)
{
    bool status = true;
    
    if (pAllocation != nullptr)
    {
        m_pAllocation = pAllocation;
    }
    else
    {
        status = false;
    }

    return status;
}

void CTexture::Uninitialize(void)
{
    if (m_pAllocation != nullptr)
    {
        m_pAllocation->Uninitialize();
        delete m_pAllocation;
        m_pAllocation = nullptr;
    }
}
