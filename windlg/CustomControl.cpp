#include <system_error>
#include "CustomControl.h"
#include <VsStyle.h>
#include <Uxtheme.h>
#pragma comment(lib, "UxTheme.lib")
using namespace lib;

void CustomControl::create(Dialog* parent, CreateOpts createOpts) const
{
	if (hWnd()) [[unlikely]] {
		throw std::logic_error("Cannot create CustomControl twice");
	}

	HINSTANCE hInst = reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(parent->hWnd(), GWLP_HINSTANCE));
	WNDCLASSEXW wcx = {
		.cbSize = sizeof(WNDCLASSEXW),
		.style = CS_DBLCLKS,
		.lpfnWndProc = _WndProc,
		.hInstance = hInst,
		.hCursor = LoadCursorW(nullptr, IDC_ARROW),
		.hbrBackground = reinterpret_cast<HBRUSH>(static_cast<DWORD_PTR>(createOpts.bgColor) + 1),
	};

	WCHAR uniqueClassName[80] = {L'\0'};
	wsprintfW(uniqueClassName, L"WNDCLASS.%Ix.%Ix.%Ix.%Ix.%Ix.%Ix.%Ix.%Ix.%Ix",
		wcx.style, wcx.cbClsExtra, wcx.cbWndExtra, wcx.hInstance, wcx.hIcon,
		wcx.hCursor, wcx.hbrBackground, wcx.lpszMenuName, wcx.hIconSm);
	wcx.lpszClassName = uniqueClassName;

	ATOM atom = RegisterClassExW(&wcx);
	if (!atom) {
		DWORD err = GetLastError();
		if (err == ERROR_CLASS_ALREADY_EXISTS) {
			atom = static_cast<ATOM>(GetClassInfoExW(hInst, wcx.lpszClassName, &wcx));
		} else {
			throw std::system_error(err, std::system_category(), "RegisterClassEx failed");
		}
	}

	if (!CreateWindowExW(createOpts.exStyle, MAKEINTATOM(atom), nullptr, createOpts.style,
			createOpts.x, createOpts.y, createOpts.cx, createOpts.cy,
			parent->hWnd(), reinterpret_cast<HMENU>(createOpts.ctrlId), hInst,
			reinterpret_cast<LPVOID>(const_cast<CustomControl*>(this)))) [[unlikely]] {
		throw std::system_error(GetLastError(), std::system_category(), "CreateWindowEx failed");
	}
}

LRESULT CALLBACK CustomControl::_WndProc(HWND hWnd, UINT uMsg, WPARAM wp, LPARAM lp)
{
	CustomControl* pSelf = nullptr;

	if (uMsg == WM_NCCREATE) {
		CREATESTRUCTW* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
		pSelf = reinterpret_cast<CustomControl*>(cs->lpCreateParams);
		SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pSelf));
		*pSelf->_hWndPtr() = hWnd;
	} else [[likely]] {
		pSelf = reinterpret_cast<CustomControl*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
	}

	if (!pSelf) [[unlikely]] {
		// No pointer stored, nothing is done.
		// Prevents processing before WM_NCCREATE and after WM_NCDESTROY.
		return DefWindowProcW(hWnd, uMsg, wp, lp);
	}

	LRESULT userRet = pSelf->wndProc(uMsg, wp, lp);

	if (uMsg == WM_NCPAINT) {
		pSelf->_paintControlBorders(wp, lp);
		userRet = 0;
	} else if (uMsg == WM_NCDESTROY) {
		SetWindowLongPtrW(hWnd, GWLP_USERDATA, 0);
		*pSelf->_hWndPtr() = nullptr;
		userRet = 0;
	}
	return userRet;
}

void CustomControl::_paintControlBorders(WPARAM wp, LPARAM lp)
{
	DefWindowProcW(hWnd(), WM_NCPAINT, wp, lp); // let the system draw the scrollbar for us

	DWORD exStyle = static_cast<DWORD>(GetWindowLongPtrW(hWnd(), GWL_EXSTYLE));
	if (!(exStyle & WS_EX_CLIENTEDGE) || !IsThemeActive() || !IsAppThemed())
		return;
	
	RECT rc{};
	GetWindowRect(hWnd(), &rc); // window outmost coordinates, including margins
	ScreenToClient(hWnd(), reinterpret_cast<POINT*>(&rc));
	ScreenToClient(hWnd(), reinterpret_cast<POINT*>(&rc.right));
	OffsetRect(&rc, 2, 2); // because it comes up anchored at -2,-2

	RECT rc2{}; // clipping region; will draw only within this rectangle
	HDC hdc = GetWindowDC(hWnd());
	if (HTHEME htd = OpenThemeData(hWnd(), L"LISTVIEW"); htd) [[likely]] {
		SetRect(&rc2, rc.left, rc.top, rc.left + 2, rc.bottom); // draw only the borders to avoid flickering
		DrawThemeBackground(htd, hdc, LVP_LISTGROUP, 0, &rc, &rc2); // draw themed left border
		SetRect(&rc2, rc.left, rc.top, rc.right, rc.top + 2);
		DrawThemeBackground(htd, hdc, LVP_LISTGROUP, 0, &rc, &rc2); // draw themed top border
		SetRect(&rc2, rc.right - 2, rc.top, rc.right, rc.bottom);
		DrawThemeBackground(htd, hdc, LVP_LISTGROUP, 0, &rc, &rc2); // draw themed right border
		SetRect(&rc2, rc.left, rc.bottom - 2, rc.right, rc.bottom);
		DrawThemeBackground(htd, hdc, LVP_LISTGROUP, 0, &rc, &rc2); // draw themed bottom border
		CloseThemeData(htd);
	}
	ReleaseDC(hWnd(), hdc);
}
