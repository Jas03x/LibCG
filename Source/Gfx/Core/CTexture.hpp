#ifndef CG_CTEXTURE_HPP
#define CG_CTEXTURE_HPP

#include "CgGfx.hpp"

class CAllocation;

struct CTexture : public ITexture
{
private:
    CAllocation* m_pAllocation;

public:
    CTexture(void);
    ~CTexture(void);

    bool Initialize(CAllocation* pAllocation);
    void Uninitialize(void);
};

#endif // CG_CTEXTURE_HPP