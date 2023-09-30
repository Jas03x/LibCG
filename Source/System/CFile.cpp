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

bool File::Read(const wchar_t* Path, byte** ppBuffer, uint32_t* pSize)
{
	bool   status = true;
	CFile* pFile = nullptr;

	if ((ppBuffer == nullptr) || (pSize == nullptr))
	{
		status = false;
		Console::Write(L"Error: Invalid or null buffer/size parameters\n");
	}

	if (status)
	{
		pFile = CFile::Open(Path);
		if (pFile == nullptr)
		{
			status = false;
		}
	}

	if (status)
	{
		*pSize = pFile->GetSize();

		if (*pSize == 0)
		{
			status = false;
		}
	}

	if (status)
	{
		*ppBuffer = reinterpret_cast<BYTE*>(Memory::Allocate(*pSize, true));
		if (*ppBuffer == nullptr)
		{
			status = false;
			Console::Write(L"Error: Could not allocate buffer to read file\n");
		}
	}

	if (status)
	{
		status = pFile->ReadBytes(*ppBuffer, *pSize);

		if (!status)
		{
			Memory::Release(*ppBuffer);
			*ppBuffer = nullptr;
			*pSize = 0;
		}
	}

	if (pFile != nullptr)
	{
		File::Close(pFile);
		pFile = nullptr;
	}

	return status;
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
	return CFile::ReadBytes(reinterpret_cast<uint8_t*>(pInt8), sizeof(int8_t));
}

bool CFile::Read(int16_t* pInt16)
{
	return CFile::ReadBytes(reinterpret_cast<uint8_t*>(pInt16), sizeof(int16_t));
}

bool CFile::Read(int32_t* pInt32)
{
	return CFile::ReadBytes(reinterpret_cast<uint8_t*>(pInt32), sizeof(int32_t));
}

bool CFile::Read(int64_t* pInt64)
{
	return CFile::ReadBytes(reinterpret_cast<uint8_t*>(pInt64), sizeof(int64_t));
}

bool CFile::Read(uint8_t* pUInt8)
{
	return CFile::ReadBytes(reinterpret_cast<uint8_t*>(pUInt8), sizeof(uint8_t));
}

bool CFile::Read(uint16_t* pUInt16)
{
	return CFile::ReadBytes(reinterpret_cast<uint8_t*>(pUInt16), sizeof(uint16_t));
}

bool CFile::Read(uint32_t* pUInt32)
{
	return CFile::ReadBytes(reinterpret_cast<uint8_t*>(pUInt32), sizeof(uint32_t));
}

bool CFile::Read(uint64_t* pUInt64)
{
	return CFile::ReadBytes(reinterpret_cast<uint8_t*>(pUInt64), sizeof(uint64_t));
}

bool CFile::ReadBytes(uint8_t* pBuffer, uint32_t numBytes)
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
		Console::Write(L"Error: Could not read %u bytes from file\n", numBytes);
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
