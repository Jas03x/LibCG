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

					if (m_pFile->ReadBytes(&block_header + sizeof(uint32_t), sizeof(MDL_BLOCK_HEADER) - sizeof(uint32_t))) // read the rest of the header
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
		if (rBlockHeader.signature == MDL_NODE)
		{
		}
	}

	return status;
}
