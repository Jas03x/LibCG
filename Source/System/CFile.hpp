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
	virtual bool Read(int8_t* pInt8);
	virtual bool Read(int16_t* pInt16);
	virtual bool Read(int32_t* pInt32);
	virtual bool Read(int64_t* pInt64);
	virtual bool Read(uint8_t* pUInt8);
	virtual bool Read(uint16_t* pUInt16);
	virtual bool Read(uint32_t* pUInt32);
	virtual bool Read(uint64_t* pUInt64);

	virtual uint64_t GetSize(void);

	virtual bool ReadBytes(void* pBuffer, uint32_t numBytes);
};

#endif // CG_CFILE_HPP