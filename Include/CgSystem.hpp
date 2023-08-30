#ifndef CG_SYS__HPP
#define CG_SYS__HPP

#include "CgDef.hpp"

#undef CreateWindow

typedef struct HWND__* HWND;

enum WIN_AREA
{
	CLIENT = 0,
	WINDOW = 1
};

struct WIN_RECT
{
	uint32_t x;
	uint32_t y;
	uint32_t width;
	uint32_t height;
};

enum WIN_MSG
{
	INVALID = 0,
	QUIT = 1
};

struct WIN_EVENT
{
	WIN_MSG msg;
};

// Shared structure between system and gfx
#ifndef CG_RENDER_BUFFER
#define CG_RENDER_BUFFER
struct RenderBuffer
{
	HANDLE hResource;
	UINT64 CpuDescriptor;
};
#endif // CG_RENDER_BUFFER

class __declspec(novtable) IWindow
{
public:
	virtual HWND         GetHandle(void) = 0;
	virtual bool         Open(void) = 0;
	virtual bool         GetEvent(WIN_EVENT& rEvent) = 0;
	virtual bool         GetRect(WIN_AREA area, WIN_RECT& rRect) = 0;

	virtual bool         Present(void) = 0;
	virtual RenderBuffer GetCurrentRenderBuffer(void) = 0;
};

class __declspec(novtable) WindowFactory
{
public:
	static IWindow* CreateInstance(const wchar_t* ClassName, const wchar_t* WindowName, uint32_t ClientWidth, uint32_t ClientHeight);
	static void     DestroyInstance(IWindow* pIWindow);
};

#endif // CG_SYS__HPP