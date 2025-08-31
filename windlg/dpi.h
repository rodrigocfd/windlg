#pragma once
#include <optional>
#include <Windows.h>

namespace lib::dpi {

// Returns the current horizontal DPI.
[[nodiscard]] UINT cx(UINT cx);

// Returns the current vertical DPI.
[[nodiscard]] UINT cy(UINT cy);

// Converts the HIMETRIC horizontal value to pixels.
// If hdc is provided, it is used.
// If hWnd is provided, retrieves its HDC and uses it.
// If neither hdc nor hWnd are provided, gets the HDC of the entire screen and uses it.
[[nodiscard]] int himetricToPixelX(int x, std::optional<HDC> hdc, std::optional<HWND> hWnd);

// Converts the HIMETRIC vertical value to pixels.
// If hdc is provided, it is used.
// If hWnd is provided, retrieves its HDC and uses it.
// If neither hdc nor hWnd are provided, gets the HDC of the entire screen and uses it.
[[nodiscard]] int himetricToPixelY(int y, std::optional<HDC> hdc, std::optional<HWND> hWnd);

// Converts the pixels horizontal value to HIMETRIC.
// If hdc is provided, it is used.
// If hWnd is provided, retrieves its HDC and uses it.
// If neither hdc nor hWnd are provided, gets the HDC of the entire screen and uses it.
[[nodiscard]] int pixelToHimetricX(int x, std::optional<HDC> hdc, std::optional<HWND> hWnd);

// Converts the pixels vertical value to HIMETRIC.
// If hdc is provided, it is used.
// If hWnd is provided, retrieves its HDC and uses it.
// If neither hdc nor hWnd are provided, gets the HDC of the entire screen and uses it.
[[nodiscard]] int pixelToHimetricY(int y, std::optional<HDC> hdc, std::optional<HWND> hWnd);

// Returns a new POINT multiplying its x and y values by the current DPI.
[[nodiscard]] POINT pt(POINT p);

// Returns a new SIZE multiplying its cx and cy values by the current DPI.
[[nodiscard]] SIZE sz(SIZE s);

// Returns the value multiplied by the current horizontal DPI.
[[nodiscard]] int x(int x);

// Returns the value multiplied by the current vertical DPI.
[[nodiscard]] int y(int y);

}
