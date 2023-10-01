#ifndef CG_IMPORTER_HPP
#define CG_IMPORTER_HPP

#include "CgImporter.hpp"

class MDL_Importer
{
private:
	class File*         m_pFile;
	Importer::MDL_DATA& m_rData;

public:
	MDL_Importer(Importer::MDL_DATA& rData);
	~MDL_Importer(void);

	bool Read(const wchar_t* path);

private:
	bool ReadSignature(uint32_t expected_signature);

	bool ReadBlock(struct MDL_BLOCK_HEADER& rBlockHeader);
	bool ReadNodeBlock(struct MDL_BLOCK_HEADER& rBlockHeader);
	bool ReadBoneBlock(struct MDL_BLOCK_HEADER& rBlockHeader);
	bool ReadMeshBlock(MDL_BLOCK_HEADER& rBlockHeader);

	bool ReadBone(void);
	bool ReadNode(void);
	bool ReadMesh(void);

	bool ReadVertexList(std::vector<Importer::Vertex>& rVertices);

	bool ReadString(std::string& rString);
	bool ReadMatrix(float* pMatrix);
};

#endif // CG_IMPORTER_HPP