#ifndef CG_CFILE_HPP
#define CG_CFILE_HPP

#include "Cg.hpp"
#include "CgDef.hpp"

class CFile : public File
{
private:
	HANDLE hFile;

private:
	CFile();
	~CFile();

public:
	static CFile* Open(const wchar_t* Path);
	static void   Close(CFile* pCFile);

public:
	virtual bool  Read(byte** ppBuffer, uint32_t* pSize);
};

#endif // CG_CFILE_HPP