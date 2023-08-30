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

	static File*  Open(const wchar_t* Path);
	static File*  Open(const FILE_PATH& Path);

	static void   Close(File* pIFile);

	virtual bool  Read(byte** ppBuffer, uint32_t* pSize) = 0;
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