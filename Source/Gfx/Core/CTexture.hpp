#ifndef CG_CTEXTURE_HPP
#define CG_CTEXTURE_HPP

#include "CgGfx.hpp"

struct ID3D12Resource;

struct CTexture : public ITexture
{
private:
    ID3D12Resource* m_pID3D12TextureBuffer;

public:
    CTexture(void);
    ~CTexture(void);

    bool Initialize(ID3D12Resource* pID3D12TextureBuffer);
    void Uninitialize(void);
};

#endif // CG_CTEXTURE_HPP