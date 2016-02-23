#include "gk2_window.h"
#include "gk2_exceptions.h"
#include <basetsd.h>

using namespace std;
using namespace gk2;

wstring Window::m_windowClassName = L"DirectX 11 Window";
int Window::m_defaultWindowWidth = 1280;
int Window::m_defaultWindowHeight = 720;

void Window::RegisterWindowClass(HINSTANCE hInstance)
{
	WNDCLASSEXW c;
	ZeroMemory(&c, sizeof(WNDCLASSEXW));

	c.cbSize = sizeof(WNDCLASSEXW);
	c.style = CS_HREDRAW | CS_VREDRAW;
	c.lpfnWndProc = WndProc;
	c.hInstance = hInstance;
	c.hCursor = LoadCursor(NULL, IDC_ARROW);
	c.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	c.lpszMenuName = NULL;
	c.lpszClassName = m_windowClassName.c_str();
	c.cbWndExtra = sizeof(LONG_PTR);
	if (!RegisterClassExW(&c))
		THROW_WINAPI;
}
bool Window::IsWindowClassRegistered(HINSTANCE hInstance)
{
	WNDCLASSEXW c;
	return GetClassInfoExW(hInstance, m_windowClassName.c_str(), &c) == TRUE;
}

Window::Window(HINSTANCE hInstance, int width, int height)
	: m_hInstance(hInstance)
{
	CreateWindowHandle(width, height, m_windowClassName);
}

Window::Window(HINSTANCE hInstance, int width, int height, const std::wstring& title)
	: m_hInstance(hInstance)
{
	CreateWindowHandle(width, height, title);
}

void Window::CreateWindowHandle(int width, int height, const wstring& title)
{
	if (!IsWindowClassRegistered(m_hInstance))
		RegisterWindowClass(m_hInstance);
	RECT rect = { 0, 0, width, height};
	DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
	if (!AdjustWindowRect(&rect, style, FALSE))
		THROW_WINAPI;
	m_hWnd = CreateWindowW(m_windowClassName.c_str(), title.c_str(), style, CW_USEDEFAULT, CW_USEDEFAULT,
		rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, m_hInstance, this);
	if (!m_hWnd)
		THROW_WINAPI;
}

Window::~Window()
{
	DestroyWindow(m_hWnd);
}

LRESULT Window::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT paintStruct;
	HDC hDC;

	switch (msg)
	{
	case WM_PAINT:
		hDC = BeginPaint(m_hWnd, &paintStruct);
		EndPaint(m_hWnd, &paintStruct);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(m_hWnd, msg, wParam, lParam);
	}
	return 0;
}

void Window::Show(int cmdShow)
{
	ShowWindow(m_hWnd, cmdShow);
}

SIZE Window::getClientSize() const
{
	RECT r;
	GetClientRect(m_hWnd, &r);
	SIZE s = { r.right - r.left, r.bottom - r.top };
	return s;
}

LRESULT CALLBACK Window::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_CREATE)
	{
		LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
		Window* wnd = (Window*)pcs->lpCreateParams;
		SetWindowLongPtrW(hWnd, GWLP_USERDATA, PtrToUlong(wnd));
		wnd->m_hWnd = hWnd;
		return wnd->WndProc(msg, wParam, lParam);
	}
	else
	{
		Window *wnd = reinterpret_cast<Window*>(static_cast<LONG_PTR>(GetWindowLongPtrW(hWnd, GWLP_USERDATA)));
		return wnd ? wnd->WndProc(msg, wParam, lParam) : DefWindowProc(hWnd, msg, wParam, lParam);
	}
}
