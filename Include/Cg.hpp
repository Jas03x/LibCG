#ifndef CG__HPP
#define CG__HPP

#include "CgDef.hpp"

int32_t CgMain(int32_t argc, const wchar_t* argv[]);

void CgAssert(bool b, const wchar_t* error, ...);

// Memory
class Memory
{
public:
	static void* Allocate(size_t nBytes, bool bClear);
	static bool  Release(void* pMemory);
};

// Console
class Console
{
public:
	static bool Write(const wchar_t* Msg, ...);
	static bool Write(const wchar_t* Msg, va_list Args);
};

// File
class File
{
public:
	enum MODE
	{
		READ  = 0,
		WRITE = 1
	};

public:
	static File*     Open(const wchar_t* Path);
	static File*     Open(const FILE_PATH& Path);

	static void      Close(File* pIFile);

	static bool      Read(const wchar_t* Path, byte** ppBuffer, uint32_t* pSize);

public:
	virtual bool     Read(int8_t*   pInt8)  = 0;
	virtual bool     Read(int16_t*  pInt16) = 0;
	virtual bool     Read(int32_t*  pInt32) = 0;
	virtual bool     Read(int64_t*  pInt64) = 0;
	virtual bool     Read(uint8_t*  pUInt8)  = 0;
	virtual bool     Read(uint16_t* pUInt16) = 0;
	virtual bool     Read(uint32_t* pUInt32) = 0;
	virtual bool     Read(uint64_t* pUInt64) = 0;

	virtual uint64_t GetSize(void) = 0;

	virtual bool     ReadBytes(uint8_t* pBuffer, uint32_t numBytes) = 0;
};

// System
class System
{
public:
	// Path
	static bool GetModulePath(wchar_t* pPath, uint32_t nSize);
	static bool GetModuleDirectory(wchar_t* pPath, uint32_t nSize);
};

#endif // Cg__HPP