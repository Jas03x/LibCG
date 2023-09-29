#ifndef CG_IMPORTER__HPP
#define CG_IMPORTER__HPP

class Importer
{
public:
	struct MDL_DATA
	{

	};

public:
	static bool ReadMdl(const wchar_t* path, MDL_DATA& rData);
};

#endif // CG_IMPORTER__HPP