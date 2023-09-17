#ifndef CG_CVERTEX_BUFFER_HPP
#define CG_CVERTEX_BUFFER_HPP

#include "CgGfx.hpp"

class CAllocation;

class CVertexBuffer : public IVertexBuffer
{
	VERTEX_BUFFER_DESC                m_Desc;

	uint64_t                          m_VertexBufferGpuVA;

	CAllocation*                      m_pAllocation;

public:
	CVertexBuffer(void);
	~CVertexBuffer(void);

	bool                              Initialize(CAllocation* pAllocation, const VERTEX_BUFFER_DESC& rDesc);
	void                              Uninitialize(void);

	uint64_t                          GetGpuVA(void);
	virtual const VERTEX_BUFFER_DESC& GetDesc(void) const;
};

#endif // CG_CVERTEX_BUFFER_HPP
