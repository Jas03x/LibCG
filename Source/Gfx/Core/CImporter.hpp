#ifndef CG_IMPORTER_HPP
#define CG_IMPORTER_HPP

#include "CgImporter.hpp"

class MDL_Importer
{
public:
	MDL_Importer(void);
	~MDL_Importer(void);

	bool Read(const wchar_t* path);

private:
	bool ReadFile(class File* pFile);
};

#endif // CG_IMPORTER_HPP