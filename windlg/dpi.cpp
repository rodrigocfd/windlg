#include "dpi.h"

static int _dpiX = 0, _dpiY = 0;

static void _cacheDpi()
{
	if (!_dpiX) { // not cached yet?
		HDC hdcScreen = GetDC(nullptr);
		_dpiX = GetDeviceCaps(hdcScreen, LOGPIXELSX);
		_dpiY = GetDeviceCaps(hdcScreen, LOGPIXELSY);
		ReleaseDC(nullptr, hdcScreen);
	}
}

UINT lib::dpi::cx(UINT cx)
{
	_cacheDpi();
	return MulDiv(cx, _dpiX, 96);
}

UINT lib::dpi::cy(UINT cy)
{
	_cacheDpi();
	return MulDiv(cy, _dpiY, 96);
}

int lib::dpi::himetricToPixelX(int x, std::optional<HDC> hdc, std::optional<HWND> hWnd)
{
	if (hdc.has_value()) {
		return MulDiv(x, GetDeviceCaps(hdc.value(), LOGPIXELSX), 2540);
	} else if (hWnd.has_value()) {
		HDC hdc = GetDC(hWnd.value());
		int pxX =  MulDiv(x, GetDeviceCaps(hdc, LOGPIXELSX), 2540);
		ReleaseDC(hWnd.value(), hdc);
		return pxX;
	} else {
		HDC hdc = GetDC(nullptr); // entire screen
		int pxX =  MulDiv(x, GetDeviceCaps(hdc, LOGPIXELSX), 2540);
		ReleaseDC(nullptr, hdc);
		return pxX;
	}
}

int lib::dpi::himetricToPixelY(int y, std::optional<HDC> hdc, std::optional<HWND> hWnd)
{
	if (hdc.has_value()) {
		return MulDiv(y, GetDeviceCaps(hdc.value(), LOGPIXELSY), 2540);
	} else if (hWnd.has_value()) {
		HDC hdc = GetDC(hWnd.value());
		int pxY =  MulDiv(y, GetDeviceCaps(hdc, LOGPIXELSY), 2540);
		ReleaseDC(hWnd.value(), hdc);
		return pxY;
	} else {
		HDC hdc = GetDC(nullptr); // entire screen
		int pxY =  MulDiv(y, GetDeviceCaps(hdc, LOGPIXELSY), 2540);
		ReleaseDC(nullptr, hdc);
		return pxY;
	}
}

int lib::dpi::pixelToHimetricX(int x, std::optional<HDC> hdc, std::optional<HWND> hWnd)
{
	if (hdc.has_value()) {
		return MulDiv(x, 2540, GetDeviceCaps(hdc.value(), LOGPIXELSX));
	} else if (hWnd.has_value()) {
		HDC hdc = GetDC(hWnd.value());
		int pxX =  MulDiv(x, 2540, GetDeviceCaps(hdc, LOGPIXELSX));
		ReleaseDC(hWnd.value(), hdc);
		return pxX;
	} else {
		HDC hdc = GetDC(nullptr); // entire screen
		int pxX =  MulDiv(x, 2540, GetDeviceCaps(hdc, LOGPIXELSX));
		ReleaseDC(nullptr, hdc);
		return pxX;
	}
}

int lib::dpi::pixelToHimetricY(int y, std::optional<HDC> hdc, std::optional<HWND> hWnd)
{
	if (hdc.has_value()) {
		return MulDiv(y, 2540, GetDeviceCaps(hdc.value(), LOGPIXELSY));
	} else if (hWnd.has_value()) {
		HDC hdc = GetDC(hWnd.value());
		int pxY =  MulDiv(y, 2540, GetDeviceCaps(hdc, LOGPIXELSY));
		ReleaseDC(hWnd.value(), hdc);
		return pxY;
	} else {
		HDC hdc = GetDC(nullptr); // entire screen
		int pxY =  MulDiv(y, 2540, GetDeviceCaps(hdc, LOGPIXELSY));
		ReleaseDC(nullptr, hdc);
		return pxY;
	}
}

POINT lib::dpi::pt(POINT p)
{
	_cacheDpi();
	return POINT{
		.x = MulDiv(p.x, _dpiX, 96),
		.y = MulDiv(p.y, _dpiY, 96),
	};
}

SIZE lib::dpi::sz(SIZE s)
{
	_cacheDpi();
	return SIZE{
		.cx = MulDiv(s.cx, _dpiX, 96),
		.cy = MulDiv(s.cy, _dpiY, 96),
	};
}

int lib::dpi::x(int x)
{
	_cacheDpi();
	return MulDiv(x, _dpiX, 96);
}

int lib::dpi::y(int y)
{
	_cacheDpi();
	return MulDiv(y, _dpiY, 96);
}
