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

INT main(INT argc, const wchar_t* argv[])
{
	INT Status = STATUS::SUCCESS;

	if (CgInitialize(argc, argv))
	{
		Status = CgMain(argc, argv);
	}
	else
	{
		Status = STATUS::UNSUCCESSFUL;
	}

	if (!CgUninitialize())
	{
		if (Status == STATUS::SUCCESS)
		{
			Status = STATUS::UNSUCCESSFUL;
		}
	}

	return Status;
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
