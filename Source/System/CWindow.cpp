#include "CWindow.hpp"

#include <strsafe.h>

#include "CSwapChain.hpp"

#undef CreateWindow

IWindow* WindowFactory::CreateInstance(const wchar_t* ClassName, const wchar_t* WindowName, uint32_t ClientWidth, uint32_t ClientHeight)
{
	CWindow* pWindow = new CWindow();

	if (pWindow != NULL)
	{
		if (!pWindow->Initialize(ClassName, WindowName, ClientWidth, ClientHeight))
		{
			WindowFactory::DestroyInstance(pWindow);
			pWindow = NULL;
		}
	}

	return pWindow;
}

void WindowFactory::DestroyInstance(IWindow* pIWindow)
{
	CWindow* pWindow = static_cast<CWindow*>(pIWindow);
	if (pWindow != NULL)
	{
		pWindow->Uninitialize();

		delete pWindow;
		pWindow = NULL;
	}
}

CWindow::CWindow(void)
{
	m_hCID = 0;
	m_bOpen = false;
	m_hWnd = NULL;
	m_hInstance = NULL;
	
	ZeroMemory(m_ClassName, sizeof(m_ClassName));
}

CWindow::~CWindow(void)
{
}

bool CWindow::Initialize(const wchar_t* ClassName, const wchar_t* WindowName, uint32_t ClientWidth, uint32_t ClientHeight)
{
	bool status = true;

	m_hInstance = GetModuleHandle(NULL);
	StringCchCopy(m_ClassName, sizeof(m_ClassName) / sizeof(wchar_t), ClassName);

	WNDCLASSEX wndClassEx = { 0 };
	wndClassEx.cbSize = sizeof(WNDCLASSEX);
	wndClassEx.style = CS_HREDRAW | CS_VREDRAW;
	wndClassEx.lpfnWndProc = WindowProcedure;
	wndClassEx.cbClsExtra = 0;
	wndClassEx.cbWndExtra = 0;
	wndClassEx.hInstance = m_hInstance;
	wndClassEx.hIcon = NULL;
	wndClassEx.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClassEx.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndClassEx.lpszMenuName = NULL;
	wndClassEx.lpszClassName = ClassName;
	wndClassEx.hIconSm = NULL;

	HWND hDesktopWindow = GetDesktopWindow();

	RECT wndRect = { };
	GetClientRect(hDesktopWindow, &wndRect);
	wndRect.left = ((wndRect.right - wndRect.left) - ClientWidth) / 2;
	wndRect.top = ((wndRect.bottom - wndRect.top) - ClientHeight) / 2;
	wndRect.right = (wndRect.left + ClientWidth);
	wndRect.bottom = (wndRect.top + ClientHeight);

	// The window size includes the borders, top bar, etc.
	// The client area is the area with the contents of the window.
	// We use AdjustWindowRect to calculate the window size based off the required client size.
	if (!AdjustWindowRect(&wndRect, WS_OVERLAPPEDWINDOW, FALSE))
	{
		status = false;
		Console::Write(L"Error: Could not calculate window bounds\n");
	}

	if (status)
	{
		m_hCID = RegisterClassEx(&wndClassEx);

		if (m_hCID == 0)
		{
			status = false;
			Console::Write(L"Error: Could not register class\n");
		}
	}

	if (status)
	{
		m_hWnd = CreateWindowEx(0, ClassName, WindowName, WS_OVERLAPPEDWINDOW, wndRect.left, wndRect.top, wndRect.right - wndRect.left, wndRect.bottom - wndRect.top, NULL, NULL, m_hInstance, NULL);

		if (m_hWnd == NULL)
		{
			status = false;
			Console::Write(L"Error: Could not create window\n");
		}
	}

	if (status)
	{
		SetWindowLongPtr(m_hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
	}

	if (status)
	{
		m_bOpen = true;
		ShowWindow(m_hWnd, SW_SHOW);
	}

	return status;
}

void CWindow::Uninitialize(void)
{
	if (m_pSwapChain != NULL)
	{
		Console::Write(L"Warning: Window being uninitialized but swap chain not released\n");
	}

	if (m_hWnd != NULL)
	{
		DestroyWindow(m_hWnd);
		m_hWnd = NULL;
	}

	UnregisterClass(m_ClassName, m_hInstance);

	m_hCID = 0;
	m_hInstance = NULL;
	m_bOpen = false;
	ZeroMemory(m_ClassName, sizeof(m_ClassName));
}

HWND CWindow::GetHandle(void)
{
	return m_hWnd;
}

bool CWindow::Open(void)
{
	return m_bOpen;
}

LRESULT CWindow::WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT Result = 0;
	CWindow* pWindow = reinterpret_cast<CWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	switch (message)
	{
		case WM_CLOSE:
		{
			pWindow->m_bOpen = false;
			break;
		}

		case WM_DESTROY:
		{
			PostQuitMessage(0);
			break;
		}

		default:
		{
			Result = DefWindowProc(hWnd, message, wParam, lParam);
			break;
		}
	}

	return Result;
}

bool CWindow::GetEvent(WIN_EVENT& rEvent)
{
	bool status = false;
	MSG msg = { 0 };

	rEvent.msg = WIN_MSG::INVALID;

	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != FALSE)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if (msg.message == WM_QUIT)
		{
			rEvent.msg = WIN_MSG::QUIT;
		}
	}

	return status;
}

bool CWindow::GetRect(WIN_AREA area, WIN_RECT& rRect)
{
	bool status = true;
	RECT rect = {};

	if (area == WIN_AREA::CLIENT)
	{
		status = GetClientRect(m_hWnd, &rect);
	}
	else
	{
		status = GetWindowRect(m_hWnd, &rect);
	}

	if (status)
	{
		rRect.x = rect.left;
		rRect.y = rect.top;
		rRect.width = rect.right - rect.left;
		rRect.height = rect.bottom - rect.top;
	}

	return status;
}

bool CWindow::SwapChainNotification(SWAPCHAIN_NOTIFICATION Notification, HANDLE hSwapChain)
{
	bool status = true;

	switch (Notification)
	{
		case SWAPCHAIN_CREATED:
		{
			m_pSwapChain = reinterpret_cast<CSwapChain*>(hSwapChain);
			break;
		}
		case SWAPCHAIN_DESTROYED:
		{
			m_pSwapChain = NULL;
			break;
		}
		default:
		{
			status = false;
			Console::Write(L"Error: Invalid swap chain notification event\n");
			break;
		}
	}

	return status;
}

bool CWindow::Present(void)
{
	bool status = true;
	status = m_pSwapChain->Present();
	return status;
}

RenderBuffer CWindow::GetCurrentRenderBuffer(void)
{
	RenderBuffer Buffer = {};
	m_pSwapChain->GetCurrentRenderBuffer(Buffer.hResource, Buffer.CpuDescriptor);
	return Buffer;
}
