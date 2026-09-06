#include "win-error.hpp"
#include "string.hpp"
#include <Uxtheme.h>
#include <vsstyle.h>
using namespace wd;

#ifdef _MSC_VER
#pragma comment(lib, "uxtheme.lib")
#endif

static std::wstring format_error_message(DWORD code, const WCHAR *reason) {
	WCHAR *pBuf = nullptr;
	const DWORD nChars = FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, code, LANG_USER_DEFAULT, reinterpret_cast<WCHAR*>(&pBuf), 0, nullptr);
	if (!nChars) [[unlikely]] {
		return L"Unknown error.";
	}
	std::wstring errMsg{pBuf};
	LocalFree(pBuf);
	str::trim_right(errMsg, L"\r\n");

	std::wstring finalMsg{};
	if (code > 0xffff) { // HRESULT
		finalMsg = (reason && lstrlenW(reason))
			? str::fmt(L"[0x%04x_%04x] %s\n%s", HIWORD(code), LOWORD(code), errMsg, reason)
			: str::fmt(L"[0x%04x_%04x] %s", HIWORD(code), LOWORD(code), errMsg);
	} else { // ordinary Win32 error code
		finalMsg = (reason && lstrlenW(reason))
			? str::fmt(L"[0x%04x %u] %s\n%s", code, code, errMsg, reason)
			: str::fmt(L"[0x%04x %u] %s", code, code, errMsg);
	}
	return finalMsg;
}

static void display_error_if_debug([[maybe_unused]] const std::wstring &msg) noexcept {
#ifdef _DEBUG
	OutputDebugStringW(L"--- WinErr constructed ---\n");
	OutputDebugStringW(msg.c_str());
	OutputDebugStringW(L"\n");
#endif
}

WinErr::WinErr(DWORD errCode, const WCHAR *reason)
	: _hr{HRESULT_FROM_WIN32(errCode)}, _msg{format_error_message(errCode, reason)}
{
	display_error_if_debug(_msg);
}

WinErr::WinErr(HRESULT hr, const WCHAR *reason)
	: _hr{hr}, _msg{format_error_message(static_cast<DWORD>(hr), reason)}
{
	display_error_if_debug(_msg);
}


std::wstring _wd_internal::get_wnd_text(HWND hWnd) {
	const size_t n = GetWindowTextLengthW(hWnd);
	std::wstring buf(n + 1, L'\0'); // alloc receiving buffer
	GetWindowTextW(hWnd, buf.data(), static_cast<int>(n) + 1);
	buf.resize(n); // remove trailing null
	return buf;
}

WORD _wd_internal::next_auto_ctrl_id() noexcept {
	static WORD nextId = 0xdfff; // https://stackoverflow.com/a/18192766
	return nextId--;
}

void _wd_internal::paint_themed_borders(HWND hWnd, WPARAM wp, LPARAM lp) noexcept {
	DefWindowProcW(hWnd, WM_NCPAINT, wp, lp); // make system draw the scrollbar for us

	const DWORD exStyle = static_cast<DWORD>(GetWindowLongPtrW(hWnd, GWL_EXSTYLE));
	if (!(exStyle & WS_EX_CLIENTEDGE) || !IsThemeActive() || !IsAppThemed()) [[unlikely]] {
		return; // no border to be painted
	}

	RECT rc{};
	GetWindowRect(hWnd, &rc); // window outmost coordinates, including margins
	ScreenToClient(hWnd, reinterpret_cast<LPPOINT>(&rc));
	ScreenToClient(hWnd, reinterpret_cast<LPPOINT>(&rc.right));
	OffsetRect(&rc, 2, 2); // because it comes up anchored at -2,-2

	const HDC hdc = GetWindowDC(hWnd);

	// The HRGN which comes in WM_NCPAINT seems to be invalid, so we carve our own.
	RECT rcHole = rc;
	InflateRect(&rcHole, -2, -2);
	const HRGN hrgnHole = CreateRectRgnIndirect(&rcHole);
	const HRGN hrgnClip = CreateRectRgnIndirect(&rc);
	CombineRgn(hrgnClip, hrgnClip, hrgnHole, RGN_DIFF);
	SelectClipRgn(hdc, hrgnClip);

	if (HTHEME hTheme = OpenThemeData(hWnd, L"EDIT"); hTheme) [[likely]] {
		DrawThemeBackground(hTheme, hdc, EP_EDITTEXT, ETS_NORMAL, &rc, nullptr);
		CloseThemeData(hTheme);
	}

	DeleteObject(hrgnClip);
	DeleteObject(hrgnHole);
	ReleaseDC(hWnd, hdc);
}
