#pragma once
#include <optional>
#include <Windows.h>

namespace lib::dpi {

[[nodiscard]] UINT cx(UINT cx);
[[nodiscard]] UINT cy(UINT cy);
[[nodiscard]] int himetricToPixelX(int x, std::optional<HDC> hdc, std::optional<HWND> hWnd);
[[nodiscard]] int himetricToPixelY(int y, std::optional<HDC> hdc, std::optional<HWND> hWnd);
[[nodiscard]] int pixelToHimetricX(int x, std::optional<HDC> hdc, std::optional<HWND> hWnd);
[[nodiscard]] int pixelToHimetricY(int y, std::optional<HDC> hdc, std::optional<HWND> hWnd);
[[nodiscard]] POINT pt(POINT p);
[[nodiscard]] SIZE sz(SIZE s);
[[nodiscard]] int x(int x);
[[nodiscard]] int y(int y);

}
