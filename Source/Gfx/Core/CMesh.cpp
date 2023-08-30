#include "CMesh.hpp"

#include "Cg.hpp"

#include <d3d12.h>

#include "CVertexBuffer.hpp"

CMesh::CMesh(void)
{
	m_pIVertexBuffer = NULL;
}

CMesh::~CMesh(void)
{

}

bool CMesh::Initialize(uint32_t NumVertices, IVertexBuffer* pIVertexBuffer)
{
	bool status = true;

	m_NumVertices = NumVertices;
	m_pIVertexBuffer = pIVertexBuffer;

	return status;
}

void CMesh::Uninitialize(void)
{
	if (m_pIVertexBuffer != NULL)
	{
		CVertexBuffer* pVertexBuffer = static_cast<CVertexBuffer*>(m_pIVertexBuffer);
		pVertexBuffer->Uninitialize();
		delete pVertexBuffer;

		m_pIVertexBuffer = NULL;
	}
}

uint32_t CMesh::GetVertexCount(void)
{
	return m_NumVertices;
}

IVertexBuffer* CMesh::GetVertexBuffer(void)
{
	return m_pIVertexBuffer;
}
