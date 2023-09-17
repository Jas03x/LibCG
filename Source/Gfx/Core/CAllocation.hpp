#ifndef CG_ALLOCATION_HPP
#define CG_ALLOCATION_HPP

#include "CgGfx.hpp"

struct ID3D12Resource;

class CAllocation
{
private:
	uint64_t        m_Offset;
	ID3D12Resource* m_pID3D12Resource;

public:
	CAllocation(void);
	~CAllocation(void);

	bool Initialize(uint64_t Offset, ID3D12Resource* pID3D12Resource);
	void Uninitialize(void);

	uint64_t GetOffset(void);
	ID3D12Resource* GetD3D12Interface(void);
};

#endif // CG_ALLOCATION_HPP