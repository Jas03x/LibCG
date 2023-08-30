#include "Cg.hpp"
#include "CConsole.hpp"

#include <cstdarg>

#include <windows.h>
#include <strsafe.h>

HANDLE   CConsole::m_hStdOut = nullptr;
wchar_t* CConsole::m_pBuffer = nullptr;

bool Console::Write(const wchar_t* Msg, ...)
{
	bool status = true;

	va_list Args;
	va_start(Args, Msg);

	status = CConsole::Write(Msg, Args);

	va_end(Args);

	return status;
}

bool Console::Write(const wchar_t* Msg, va_list Args)
{
	bool status = true;

	status = CConsole::Write(Msg, Args);

	return status;
}

bool CConsole::Initialize(void)
{
	bool status = true;

	m_hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

	if ((m_hStdOut == nullptr) || (m_hStdOut == INVALID_HANDLE_VALUE))
	{
		status = false;
	}

	if (status)
	{
		m_pBuffer = reinterpret_cast<wchar_t*>(Memory::Allocate(MaxLength * sizeof(wchar_t), true));
	}

	return status;
}

bool CConsole::Uninitialize(void)
{
	bool status = true;

	m_hStdOut = nullptr;

	if (m_pBuffer != nullptr)
	{
		Memory::Release(m_pBuffer);
		m_pBuffer = nullptr;
	}

	return status;
}

bool CConsole::Write(const wchar_t* Msg, va_list Args)
{
	bool status = true;
	size_t CharsFree = 0;
	DWORD  CharsUsed = 0;
	DWORD  CharsWritten = 0;

	if (StringCchVPrintfEx(m_pBuffer, MaxLength, NULL, &CharsFree, 0, Msg, Args) == S_OK)
	{
		CharsUsed = static_cast<DWORD>(MaxLength - CharsFree);
	}
	else
	{
		status = false;
	}

	if (status)
	{
		status = WriteConsole(m_hStdOut, m_pBuffer, CharsUsed, &CharsWritten, NULL);
	}

	if (status)
	{
		status = (CharsWritten == CharsUsed) ? true : false;
	}

	return status;
}
