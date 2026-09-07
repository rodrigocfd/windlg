#pragma once
#include <string>

#include <sdkddkver.h>
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace wd {

	/// Wraps a Win32 error or an HRESULT.
	class WinErr {
	public:
		WinErr(DWORD errCode, const WCHAR *reason);
		WinErr(DWORD errCode, const std::wstring &reason) : WinErr{errCode, reason.c_str()} { }
		explicit WinErr(DWORD errCode)                    : WinErr{errCode, nullptr} { }
		WinErr(HRESULT hr, const WCHAR *reason);
		WinErr(HRESULT hr, const std::wstring &reason)    : WinErr{hr, reason.c_str()} { }
		explicit WinErr(HRESULT hr)                       : WinErr{hr, nullptr} { }

		[[nodiscard]] constexpr HRESULT hr() const noexcept              { return _hr; }
		[[nodiscard]] constexpr const std::wstring& msg() const noexcept { return _msg; }
	private:
		HRESULT _hr;
		std::wstring _msg;
	};

}

// Utilities used internally by the library.
namespace _wd_internal {

	std::wstring get_wnd_text(HWND hWnd);
	WORD next_auto_ctrl_id() noexcept;
	void paint_themed_borders(HWND hWnd, WPARAM wp, LPARAM lp) noexcept;

}
