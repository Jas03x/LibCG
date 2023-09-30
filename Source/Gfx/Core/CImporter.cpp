#include "CImporter.hpp"

#include "Cg.hpp"
#include "MdlFormat.hpp"

bool Importer::ReadMdl(const wchar_t* path, MDL_DATA& rData)
{
	bool status = true;

	MDL_Importer importer(rData);

	if (importer.Read(path))
	{

	}
	else
	{
		status = false;
	}

	return status;
}

MDL_Importer::MDL_Importer(Importer::MDL_DATA& rData) : m_rData(rData)
{

}

MDL_Importer::~MDL_Importer(void)
{

}

bool MDL_Importer::ReadSignature(uint32_t expected_signature)
{
	bool status = true;

	uint32_t signature = 0;

	if (m_pFile->Read(&signature))
	{
		if (signature != expected_signature)
		{
			Console::Write(L"Error: Invalid or corrupted MDL file\n");
			status = false;
		}
	}
	else
	{
		status = false;
	}

	return status;
}

bool MDL_Importer::Read(const wchar_t* path)
{
	bool status = true;

	m_pFile = File::Open(path);

	if (m_pFile == nullptr)
	{
		status = false;
	}

	if (status)
	{
		MDL_HEADER mdl_header;

		if (m_pFile->ReadBytes(&mdl_header, sizeof(MDL_HEADER)))
		{
			if (mdl_header.signature != MDL_SIG)
			{
				Console::Write(L"Error: Invalid MDL signature\n");
				status = false;
			}
		}
		else
		{
			status = false;
		}
	}

	if (status)
	{
		uint32_t next_uint32 = 0;
		while (true)
		{
			if (m_pFile->Read(&next_uint32))
			{
				if (next_uint32 == MDL_BLOCK)
				{
					MDL_BLOCK_HEADER block_header = { 0 };
					block_header.signature = next_uint32;

					if (m_pFile->ReadBytes(reinterpret_cast<uint8_t*>(&block_header) + sizeof(uint32_t), sizeof(MDL_BLOCK_HEADER) - sizeof(uint32_t))) // read the rest of the header
					{
						status = ReadBlock(block_header);
					}
					else
					{
						status = false;
					}
				}
				else
				{
					break;
				}
			}
			else
			{
				status = false;
				break;
			}
		}

		if (next_uint32 != MDL_EOF)
		{
			Console::Write(L"Error: Expected MDL EOF\n");
			status = false;
		}
	}

	if (m_pFile != nullptr)
	{
		File::Close(m_pFile);
		m_pFile = nullptr;
	}

	return status;
}

bool MDL_Importer::ReadBlock(MDL_BLOCK_HEADER& rBlockHeader)
{
	bool status = true;

	if (status)
	{
		switch (rBlockHeader.signature)
		{
			case MDL_NODE:
			{
				status = ReadNodeBlock(rBlockHeader);
				break;
			}
		}
	}

	return status;
}

bool MDL_Importer::ReadNodeBlock(MDL_BLOCK_HEADER& rBlockHeader)
{
	bool status = true;

	MDL_LIST_HEADER list_header = { 0 };

	if (!m_pFile->ReadBytes(&list_header, sizeof(MDL_LIST_HEADER)))
	{
		status = false;
	}

	if (status)
	{
		if (list_header.signature != MDL_LIST)
		{
			Console::Write(L"Error: Expected MDL list\n");
			status = false;
		}
		else if (list_header.type != MDL_NODE)
		{
			Console::Write(L"Error: Expected node list\n");
			status = false;
		}
	}

	if (status)
	{
		for (uint32_t i = 0; status && (i < list_header.length); i++)
		{
			status = ReadNode();
		}
	}

	if (status)
	{
		status = ReadSignature(MDL_END);
	}

	return status;
}

bool MDL_Importer::ReadNode(void)
{
	bool status = true;
	uint64_t node_signature = 0;

	m_rData.nodes.push_back(Importer::Node());
	Importer::Node& rNode = m_rData.nodes.back();

	if (m_pFile->Read(&node_signature))
	{
		if (node_signature != MDL_NODE)
		{
			Console::Write(L"Error: Expected node\n");
			status = false;
		}
	}
	else
	{
		status = false;
	}

	if (status)
	{
		status = ReadString(rNode.name);
	}

	if (status)
	{
		status = ReadString(rNode.parent);
	}

	if (status)
	{
		status = ReadMatrix(rNode.matrix);
	}

	if (status)
	{
		status = ReadSignature(MDL_END);
	}

	return status;
}

bool MDL_Importer::ReadString(std::string& rString)
{
	bool status = true;

	MDL_STRING_HEADER string_header = { 0 };
	if (m_pFile->ReadBytes(&string_header, sizeof(MDL_STRING_HEADER)))
	{
		if (string_header.signature != MDL_STRING)
		{
			Console::Write(L"Error: Expected string\n");
			status = false;
		}
	}
	else
	{
		status = false;
	}

	if (status)
	{
		uint8_t  buffer[255] = { 0 };
		uint32_t length = 0;

		while(status && (length < sizeof(buffer)))
		{
			uint8_t c = 0;
			if (m_pFile->Read(&c))
			{
				buffer[length++] = c;

				if (c == 0)
				{
					break;
				}
			}
			else
			{
				status = false;
			}
		}

		if (length == string_header.length)
		{
			rString = std::string(reinterpret_cast<char*>(buffer), length);
		}
		else
		{
			Console::Write(L"Error: String size mismatch\n");
			status = false;
		}
	}

	if (status)
	{
		status = ReadSignature(MDL_END);
	}

	return status;
}

bool MDL_Importer::ReadMatrix(float* pMatrix)
{
	bool status = true;

	MDL_MATRIX_DATA matrix_data = { 0 };

	if (m_pFile->ReadBytes(&matrix_data, sizeof(MDL_MATRIX_DATA)))
	{
		if (matrix_data.signature == MDL_MATRIX4)
		{
			memcpy(pMatrix, matrix_data.elements, sizeof(matrix_data.elements));
		}
		else
		{
			Console::Write(L"Error: Expected matrix\n");
			status = false;
		}
	}
	else
	{
		status = false;
	}

	if (status)
	{
		status = ReadSignature(MDL_END);
	}

	return status;
}
