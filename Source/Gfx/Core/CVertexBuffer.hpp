#ifndef CG_CVERTEX_BUFFER_HPP
#define CG_CVERTEX_BUFFER_HPP

#include "CgGfx.hpp"

struct ID3D12Resource;

class CVertexBuffer : public IVertexBuffer
{
	VERTEX_BUFFER_DESC                m_Desc;

	uint64_t                          m_VertexBufferGpuVA;

	ID3D12Resource*                   m_pID3D12Resource;

public:
	CVertexBuffer(void);
	~CVertexBuffer(void);

	bool                              Initialize(ID3D12Resource* pID3D12VertexBuffer, const VERTEX_BUFFER_DESC& rDesc);
	void                              Uninitialize(void);

	uint64_t                          GetGpuVA(void);
	virtual const VERTEX_BUFFER_DESC& GetDesc(void) const;
};

#endif // CG_CVERTEX_BUFFER_HPP
