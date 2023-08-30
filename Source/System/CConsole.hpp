#ifndef CG_CCONSOLE_HPP
#define CG_CCONSOLE_HPP

#include "CgDef.hpp"

class CConsole
{
private:
	enum { MaxLength = 1024 };

	static HANDLE   m_hStdOut;
	static wchar_t* m_pBuffer;

public:
	static bool Initialize(void);
	static bool Uninitialize(void);

	static bool Write(const wchar_t* Msg, va_list Args);
};

#endif // CG_CCONSOLE_HPP