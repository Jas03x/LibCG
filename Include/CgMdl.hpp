#ifndef CG_MDL__HPP
#define CG_MDL__HPP

/*
* _________________________________________________
* |                MDL File Format                |
* |-----------------------------------------------|
* |    Type         |   Description               |
* |-----------------|-----------------------------|
* |    Header       |   Header                    |
* |-----------------|-----------------------------|
* |    Block        |   Block 1                   |
* |-----------------|-----------------------------|
* |    Block        |   .....                     |
* |-----------------|-----------------------------|
* |    Block        |   Block N                   |
* |-----------------|-----------------------------|
* |    UInt16       |   End of File 'EOF'         |
* |_________________|_____________________________|
*
* Header
* _________________________________________________
* |                MDL Header Format              |
* |-----------------------------------------------|
* |    Type         |  Description                |
* |-----------------|-----------------------------|
* |    Uint32       |  Signature 'MDL'            |
* |-----------------|-----------------------------|
* |    Uint32       |  Version                    |
* |-----------------|-----------------------------|
* |    Uint32[6]    |  Reserved                   |
* |_________________|_____________________________|
* 
* _________________________________________________
* |                MDL String Format              |
* |-----------------------------------------------|
* |    Type         |  Description                |
* |-----------------|-----------------------------|
* |    Uint32       |  String Signature 'STR'     |
* |-----------------|-----------------------------|
* |    UInt8        |  Length                     |
* |-----------------|-----------------------------|
* |    char[]       |  Data                       |
* |_________________|_____________________________|
* * Note: The string is null terminated, and the size includes the terminator.
*
* _________________________________________________
* |                MDL List Format                |
* |-----------------------------------------------|
* |    Type         |   Description               |
* |-----------------|-----------------------------|
* |    Uint32       |   List Signature 'LIST'     |
* |-----------------|-----------------------------|
* |    UInt32       |   Type                      |
* |-----------------|-----------------------------|
* |    UInt32       |   Length                    |
* |-----------------|-----------------------------|
* |    UInt32       |   Reserved                  |
* |-----------------|-----------------------------|
* |                Data                           |
* |-----------------|-----------------------------|
* |    Uint32       |   End of data 'END'         |
* |_________________|_____________________________|
*
* _________________________________________________
* |                MDL Matrix4F                   |
* |-----------------------------------------------|
* |    Type         |   Description               |
* |-----------------|-----------------------------|
* |    Uint32       |   Signature 'MX4'           |
* |-----------------|-----------------------------|
* |    Float[16]    |   Data                      |
* |-----------------|-----------------------------|
* |    Uint32       |   End of data 'END'         |
* |_________________|_____________________________|
*
* _________________________________________________
* |                MDL Block Format               |
* |-----------------------------------------------|
* |    Type         |   Description               |
* |-----------------|-----------------------------|
* |    Uint32       |   Block Signature 'BLK'     |
* |-----------------|-----------------------------|
* |    Uint32       |   Type                      |
* |-----------------|-----------------------------|
* |    Uint32[2]    |   Reserved                  |
* |-----------------|-----------------------------|
* |                Data                           |
* |-----------------|-----------------------------|
* |    Uint32       |   End of data 'END'         |
* |_________________|_____________________________|
* 
* _________________________________________________
* |           MDL Node Data Block Format          |
* |-----------------------------------------------|
* |    Type         |   Description               |
* |-----------------|-----------------------------|
* |    Uint32       |   Block Signature 'BLK'     |
* |-----------------|-----------------------------|
* |    Uint32       |   Type 'NODE'               |
* |-----------------|-----------------------------|
* |    Uint32[2]    |   Reserved                  |
* |-----------------|-----------------------------|
* |    MDL_Array    |   Nodes                     |
* |-----------------|-----------------------------|
* |    Uint32       |   End of data 'END'         |
* |_________________|_____________________________|
* 
* _________________________________________________
* |           MDL Bone Data Block Format          |
* |-----------------------------------------------|
* |    Type         |   Description               |
* |-----------------|-----------------------------|
* |    Uint32       |   Block Signature 'BLK'     |
* |-----------------|-----------------------------|
* |    Uint32       |   Type 'BONE'               |
* |-----------------|-----------------------------|
* |    Uint32[2]    |   Reserved                  |
* |-----------------|-----------------------------|
* |    MDL_Array    |   Bones                     |
* |-----------------|-----------------------------|
* |    Uint32       |   End of data 'END'         |
* |_________________|_____________________________|
*
* _________________________________________________
* |           MDL Material Data Block Format      |
* |-----------------------------------------------|
* |    Type         |   Description               |
* |-----------------|-----------------------------|
* |    Uint32       |   Block Signature 'BLK'     |
* |-----------------|-----------------------------|
* |    MDL_String   |   Texture                   |
* |-----------------|-----------------------------|
* |    Uint32       |   End of data 'END'         |
* |_________________|_____________________________|
*
* _________________________________________________
* |           MDL Mesh Data Block Format          |
* |-----------------------------------------------|
* |    Type         |   Description               |
* |-----------------|-----------------------------|
* |    Uint32       |   Block Signature 'BLK'     |
* |-----------------|-----------------------------|
* |    Uint32       |   Type 'MESH'               |
* |-----------------|-----------------------------|
* |    Uint32[2]    |   Reserved                  |
* |-----------------|-----------------------------|
* |    MDL_Array    |   Vertices                  |
* |-----------------|-----------------------------|
* |    MDL_Array    |   Mesh Data                 |
* |-----------------|-----------------------------|
* |    Uint32       |   End of data 'END'         |
* |_________________|_____________________________|
*
* _________________________________________________
* |                MDL Node Format                |
* |-----------------------------------------------|
* |    Type         |   Description               |
* |-----------------|-----------------------------|
* |    Uint32       |   Node Signature 'NODE'     |
* |-----------------|-----------------------------|
* |    MDL_String   |   Name                      |
* |-----------------|-----------------------------|
* |    MDL_String   |   Parent                    |
* |-----------------|-----------------------------|
* |    MDL_Matrix4F |   Transform                 |
* |-----------------|-----------------------------|
* |    Uint32       |   End of data 'END'         |
* |_________________|_____________________________|
*
* _________________________________________________
* |                MDL Bone Format                |
* |-----------------------------------------------|
* |    Type         |   Description               |
* |-----------------|-----------------------------|
* |    Uint32       |   Node Signature 'BONE'     |
* |-----------------|-----------------------------|
* |    MDL_String   |   Name                      |
* |-----------------|-----------------------------|
* |    MDL_Matrix4F |   Offset matrix             |
* |-----------------|-----------------------------|
* |    Uint32       |   End of data 'END'         |
* |_________________|_____________________________|
*
* _________________________________________________
* |                MDL Vertex Format              |
* |-----------------------------------------------|
* |    Type         |  Description                |
* |-----------------|-----------------------------|
* |    UInt16       |  Vertex Signature 'VX'      |
* |-----------------|-----------------------------|
* |    float[3]     |  Position                   |
* |-----------------|-----------------------------|
* |    float[3]     |  Normal                     |
* |-----------------|-----------------------------|
* |    float[2]     |  UV                         |
* |-----------------|-----------------------------|
* |    Uint8        |  Node index                 |
* |-----------------|-----------------------------|
* |    Uint8        |  Bone count                 |
* |-----------------|-----------------------------|
* |    Uint8[4]     |  Bone indices               |
* |-----------------|-----------------------------|
* |    float[4]     |  Bone weights               |
* |_________________|_____________________________|
*
* _________________________________________________
* |                MDL Mesh Format                |
* |-----------------------------------------------|
* |    Type         |   Description               |
* |-----------------|-----------------------------|
* |    Uint32       |   Mesh Signature 'MESH'     |
* |-----------------|-----------------------------|
* |    MDL_String   |   Name                      |
* |-----------------|-----------------------------|
* |    MDL_Array    |   Indices                   |
* |-----------------|-----------------------------|
* |    Uint32       |   End of data 'END'         |
* |_________________|_____________________________|
*
*/

#include <stdint.h>

enum
{
	MDL_SIG     = 0x004C444D, // 'MDL'
	MDL_EOF     = 0x00464F45, // 'EOF'
	MDL_LIST    = 0x5354494C, // 'LIST'
	MDL_BLOCK   = 0x004B4C42, // 'BLK'
	MDL_VERTEX  = 0x00005856, // 'VX'
	MDL_STRING  = 0x00525453, // 'STR'
	MDL_MATRIX  = 0x0034584D, // 'MX4'
	MDL_BONE    = 0x454E4F42, // 'BONE'
	MDL_NODE    = 0x45444F4E, // 'NODE'
	MDL_MTL     = 0x004C544D, // 'MTL'
	MDL_MESH    = 0x4853454D, // 'MESH'
	MDL_INDEX   = 0x58444E49, // 'INDX'
	MDL_TEXTURE = 0x54584554, // 'TEXT'
	MDL_END     = 0x00444E45  // 'END'
};

struct MDL_HEADER
{
	uint32_t signature;
	uint32_t version;
	uint32_t reserved[6];
};

struct MDL_BLOCK
{
	uint64_t signature;
	uint8_t  type;
	uint16_t reserved;
};

struct MDL_ARRAY
{
	uint64_t signature;
	uint8_t  type;
	uint32_t length;
};

struct MDL_VERTEX
{
	uint8_t signature;
	float   position[3];
	float   normal[3];
	float   uv[2];
	uint8_t node_index;
	uint8_t bone_count;
	uint8_t bone_indices[4];
	float   bone_weights[4];
};

struct MDL_STRING
{
	uint32_t signature;
	uint8_t  length;
	char     data[];
};

struct MDL_MATRIX
{
	uint8_t signature;
	float   elements[16];
};

#endif // CG_MDL__HPP