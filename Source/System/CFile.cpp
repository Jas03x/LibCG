#include "CFile.hpp"

#include <strsafe.h>
#include <windows.h>

File* File::Open(const wchar_t* Path)
{
	return static_cast<File*>(CFile::Open(Path));
}

File* File::Open(const FILE_PATH& Path)
{
	bool status = true;
	File* pFile = nullptr;
	wchar_t Buffer[1024] = {0};

	if (StringCchCatEx(Buffer, _countof(Buffer), Path.Directory, NULL, NULL, 0) == S_OK)
	{
		status = (StringCchCatEx(Buffer, _countof(Buffer), Path.FileName, NULL, NULL, 0) == S_OK) ? true : false;
	}
	else
	{
		status = false;
	}

	if (status)
	{
		pFile = File::Open(Buffer);
	}

	return pFile;
}

void File::Close(File* pIFile)
{
	return CFile::Close(static_cast<CFile*>(pIFile));
}

CFile::CFile()
{
	hFile = nullptr;
}

CFile::~CFile()
{
}

CFile* CFile::Open(const wchar_t* Path)
{
	CFile* pCFile = new CFile();

	pCFile->hFile = CreateFile(Path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	
	if (pCFile->hFile == INVALID_HANDLE_VALUE)
	{
		delete pCFile;
		pCFile = nullptr;

		Console::Write(L"Error: Could not open file %s for reading\n", Path);
	}

	return pCFile;
}

void CFile::Close(CFile* pCFile)
{
	if ((pCFile->hFile != 0) && (pCFile->hFile != INVALID_HANDLE_VALUE))
	{
		CloseHandle(pCFile->hFile);
		pCFile->hFile = nullptr;
	}

	delete pCFile;
}

bool CFile::Read(byte** ppBuffer, uint32_t* pSize)
{
	bool status = true;
	LARGE_INTEGER lpFileSize = {};

	if ((ppBuffer == nullptr) || (pSize == nullptr))
	{
		status = false;
		Console::Write(L"Error: Invalid or null buffer/size parameters\n");
	}

	if (status)
	{
		if (!GetFileSizeEx(hFile, &lpFileSize))
		{
			status = false;
			Console::Write(L"Error: Could not get file size\n");
		}
	}

	if (status)
	{
		*ppBuffer = reinterpret_cast<BYTE*>(Memory::Allocate(lpFileSize.QuadPart, true));
		if (*ppBuffer == nullptr)
		{
			status = false;
			Console::Write(L"Error: Could not allocate buffer to read file\n");
		}
	}

	if (status)
	{
		DWORD nBytesRead = 0;

		if (ReadFile(hFile, *ppBuffer, static_cast<DWORD>(lpFileSize.QuadPart), &nBytesRead, NULL))
		{
			*pSize = nBytesRead;
		}
		else
		{
			status = false;
			Console::Write(L"Error: Could read file contents\n");
		}
	}

	return status;
}
