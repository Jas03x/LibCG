#include "Cg.hpp"
#include "CSystem.hpp"

#include <windows.h>
#include <libloaderapi.h>

bool System::GetModulePath(wchar_t* pPath, uint32_t nSize)
{
	bool status = true;

	if (pPath == nullptr)
	{
		status = false;
		Console::Write(L"Error: Null path string buffer\n");
	}

	if (status)
	{
		DWORD length = GetModuleFileName(NULL, pPath, nSize);

		if ((length == 0) || ((length == nSize) && (GetLastError() == ERROR_INSUFFICIENT_BUFFER)))
		{
			status = false;
			Console::Write(L"Error 0x%X: Could not get module path\n", GetLastError());
		}
	}

	return status;
}

bool System::GetModuleDirectory(wchar_t* pPath, uint32_t nSize)
{
	bool status = true;

	status = GetModulePath(pPath, nSize);

	if (status)
	{
		uint32_t index = 0xFFFFFFFF;

		for (wchar_t* p = pPath; *p != 0; p++)
		{
			if ((*p == '/') || (*p == '\\'))
			{
				index = static_cast<uint32_t>(p - pPath);
			}
		}

		if (index != 0xFFFFFFFF)
		{
			for (uint32_t i = index; i < nSize; i++)
			{
				pPath[i] = 0;
			}
		}
	}

	return status;
}
