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
