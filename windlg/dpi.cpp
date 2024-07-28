#include "dpi.h"

int dpiX = 0, dpiY = 0;

void _cacheDpi()
{
	if (!dpiX) { // not cached yet?
		HDC hdcScreen = GetDC(nullptr);
		dpiX = GetDeviceCaps(hdcScreen, LOGPIXELSX);
		dpiY = GetDeviceCaps(hdcScreen, LOGPIXELSY);
		ReleaseDC(nullptr, hdcScreen);
	}
}

POINT lib::dpi::pt(POINT p)
{
	_cacheDpi();
	return POINT{
		.x = MulDiv(p.x, dpiX, 96),
		.y = MulDiv(p.y, dpiY, 96),
	};
}

SIZE lib::dpi::sz(SIZE s)
{
	_cacheDpi();
	return SIZE{
		.cx = MulDiv(s.cx, dpiX, 96),
		.cy = MulDiv(s.cy, dpiY, 96),
	};
}

int lib::dpi::x(int x)
{
	_cacheDpi();
	return MulDiv(x, dpiX, 96);
}

int lib::dpi::y(int y)
{
	_cacheDpi();
	return MulDiv(y, dpiY, 96);
}
