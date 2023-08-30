#ifndef CG_CCONSTANT_BUFFER_HPP
#define CG_CCONSTANT_BUFFER_HPP

#include "CgGfx.hpp"

struct ID3D12Resource;

class CConstantBuffer : public IConstantBuffer
{
private:
	ID3D12Resource* m_pID3D12Resource;

	void*    m_CpuVA;
	uint64_t m_GpuVA;

public:
	CConstantBuffer(void);
	~CConstantBuffer(void);

	bool Initialize(ID3D12Resource* pID3D12ConstantBufferResource, void* CpuVA);
	void Uninitialize(void);

	virtual void*    GetCpuVA(void);
	virtual uint64_t GetGpuVA(void);
};

#endif // CG_CCONSTANT_BUFFER_HPP
