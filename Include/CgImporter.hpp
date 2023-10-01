#ifndef CG_IMPORTER__HPP
#define CG_IMPORTER__HPP

#include <stdint.h>

#include <string>
#include <vector>

class Importer
{
public:
	struct Vertex
	{
		float       position[3];
		float       normal[3];
		float       uv[2];
		uint8_t     node_index;
		uint8_t     bone_count;
		uint8_t     bone_indices[4];
		float       bone_weights[4];
	};

	struct Bone
	{
		std::string name;
		float       offset_matrix[16];
	};

	struct Node
	{
		std::string name;
		std::string parent;
		float       matrix[16];
	};

	struct Material
	{
		std::string name;
		std::string texture;
	};

	struct Mesh
	{
		std::string           name;
		std::vector<Vertex>   vertices;
		std::vector<uint16_t> indices;
	};

	struct MDL_DATA
	{
		std::vector<Node>     nodes;
		std::vector<Bone>     bones;
		std::vector<Mesh>     meshes;
		std::vector<Material> materials;
	};

public:
	static bool ReadMdl(const wchar_t* path, MDL_DATA& rData);
};

#endif // CG_IMPORTER__HPP