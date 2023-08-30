#include "Cg.hpp"

#include <cstdarg>

#include "windows.h"

#include "System/CConsole.hpp"
#include "System/CMemory.hpp"

bool CgInitialize(int32_t argc, const wchar_t* argv[]);
bool CgUninitialize(void);

void CgAssert(bool b, const wchar_t* error, ...)
{
	if (!b)
	{
		va_list args;
		va_start(args, error);

		Console::Write(error, args);

		va_end(args);
	}
}

int32_t main(int32_t argc, const wchar_t* argv[])
{
	int32_t status = STATUS::SUCCESS;

	if (CgInitialize(argc, argv))
	{
		status = CgMain(argc, argv);
	}
	else
	{
		status = STATUS::UNSUCCESSFUL;
	}

	if (!CgUninitialize())
	{
		if (status == STATUS::SUCCESS)
		{
			status = STATUS::UNSUCCESSFUL;
		}
	}

	return status;
}

bool CgInitialize(int32_t argc, const wchar_t* argv[])
{
	bool status = true;

	if (status)
	{
		status = CMemory::Initialize();
	}

	if (status)
	{
		status = CConsole::Initialize();
	}

	return status;
}

bool CgUninitialize(void)
{
	bool status = true;

	if (!CConsole::Uninitialize())
	{
		status = false;
	}
	
	if (!CMemory::Uninitialize())
	{
		status = false;
	}

	return status;
}

PVOID operator new(size_t size)
{
	return Memory::Allocate(size, true);
}

void operator delete(void* ptr)
{
	Memory::Release(ptr);
}
