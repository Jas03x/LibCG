#ifndef CG_DEF__HPP
#define CG_DEF__HPP

typedef signed char			int8_t;
typedef signed short		int16_t;
typedef signed int			int32_t;
typedef signed long long	int64_t;
typedef unsigned char		uint8_t;
typedef unsigned short		uint16_t;
typedef unsigned int		uint32_t;
typedef unsigned long long	uint64_t;

typedef uint8_t             byte;

typedef void*               HANDLE;

typedef char*				va_list;

enum STATUS
{
	SUCCESS         = 0x00000000L,
	UNSUCCESSFUL    = 0xC0000001L,
	NOT_IMPLEMENTED = 0xC0000002L
};

typedef struct FILE_PATH
{
	const wchar_t* Directory;
	const wchar_t* FileName;

	inline FILE_PATH(const wchar_t* directory, const wchar_t* filename)
	{
		Directory = directory;
		FileName  = filename;
	}
} FILE_PATH;

enum : uint32_t
{
	KB = 1024 * sizeof(byte),
	MB = 1024 * KB,
	GB = 1024 * MB
};

#endif // CG_DEF__HPP