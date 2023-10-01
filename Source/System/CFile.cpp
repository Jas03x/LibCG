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
	CgAssert(hFile == nullptr, L"File handle not closed\n");
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

bool CFile::Read(int8_t* pInt8)
{
	return CFile::ReadBytes(pInt8, sizeof(int8_t));
}

bool CFile::Read(int16_t* pInt16)
{
	return CFile::ReadBytes(pInt16, sizeof(int16_t));
}

bool CFile::Read(int32_t* pInt32)
{
	return CFile::ReadBytes(pInt32, sizeof(int32_t));
}

bool CFile::Read(int64_t* pInt64)
{
	return CFile::ReadBytes(pInt64, sizeof(int64_t));
}

bool CFile::Read(uint8_t* pUInt8)
{
	return CFile::ReadBytes(pUInt8, sizeof(uint8_t));
}

bool CFile::Read(uint16_t* pUInt16)
{
	return CFile::ReadBytes(pUInt16, sizeof(uint16_t));
}

bool CFile::Read(uint32_t* pUInt32)
{
	return CFile::ReadBytes(pUInt32, sizeof(uint32_t));
}

bool CFile::Read(uint64_t* pUInt64)
{
	return CFile::ReadBytes(pUInt64, sizeof(uint64_t));
}

bool CFile::ReadBytes(void* pBuffer, uint32_t numBytes)
{
	bool status = true;

	DWORD bytesRead = 0;

	if (ReadFile(hFile, pBuffer, numBytes, &bytesRead, NULL) != 0)
	{
		status = false;
	}
	else
	{
		status = (bytesRead == numBytes);

		if (!status)
		{
			Console::Write(L"Error: Could not read %u bytes from file\n", numBytes);
		}
	}

	return status;
}

uint64_t CFile::GetSize(void)
{
	uint64_t size = 0;

	LARGE_INTEGER lpFileSize = {};

	if ((hFile != NULL) && (GetFileSizeEx(hFile, &lpFileSize) != 0))
	{
		size = lpFileSize.QuadPart;
	}
	else
	{
		Console::Write(L"Error: Could not get file size\n");
	}

	return size;
}
