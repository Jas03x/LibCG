#include "CImporter.hpp"

#include "Cg.hpp"

bool Importer::ReadMdl(const wchar_t* path, MDL_DATA& rData)
{
	bool status = true;

	MDL_Importer importer;

	if (importer.Read(path))
	{

	}
	else
	{
		status = false;
	}

	return status;
}

MDL_Importer::MDL_Importer(void)
{

}

MDL_Importer::~MDL_Importer(void)
{

}

bool MDL_Importer::Read(const wchar_t* path)
{
	bool status = true;

	File* pFile = File::Open(path);

	if (pFile != nullptr)
	{
		status = ReadFile(pFile);
	}
	else
	{
		status = false;
	}

	if (pFile != nullptr)
	{
		File::Close(pFile);
		pFile = nullptr;
	}

	return status;
}

bool MDL_Importer::ReadFile(File* pFile)
{
	bool status = true;



	return status;
}
