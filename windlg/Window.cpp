#include <system_error>
#include "Window.h"
using namespace lib;

std::wstring Window::text() const
{
	UINT len = GetWindowTextLengthW(_hWnd);
	if (!len) {
		DWORD err = GetLastError();
		if (err != ERROR_SUCCESS) [[unlikely]] {
			throw std::system_error(err, std::system_category(), "GetWindowTextLength failed");
		} else {
			return std::wstring();
		}
	} else {
		std::wstring buf(len + 1, L'\0');
		GetWindowTextW(hWnd(), buf.data(), len + 1);
		buf.resize(len);
		return buf;
	}
}

void Window::setText(std::wstring_view text) const
{
	if (!SetWindowTextW(_hWnd, text.data())) [[unlikely]] {
		throw std::system_error(GetLastError(), std::system_category(), "SetWindowText failed");
	}
}
