#pragma once
#include <string>
#include <Windows.h>

namespace lib {

// Simple HWND wrapper, base to all windows and controls.
class Window {
private:
	HWND _hWnd = nullptr;

public:
	virtual ~Window() { }

	constexpr Window() = default;
	constexpr Window(const Window& other) = default;
	Window(Window&& other) noexcept { operator=(std::forward<Window>(other)); }
	constexpr Window& operator=(const Window&) = default;
	Window& operator=(Window&& other) noexcept;

	constexpr explicit Window(HWND hWnd) : _hWnd{hWnd} { }

	[[nodiscard]] constexpr HWND hWnd() const { return _hWnd; }
	[[nodiscard]] std::wstring text() const;
	void setText(std::wstring_view text) const;

protected:
	[[nodiscard]] constexpr HWND* _hWndPtr() { return &_hWnd; }
};

}
