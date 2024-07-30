#include "ProgressBar.h"
using namespace lib;

UINT ProgressBar::pos() const
{
	return static_cast<UINT>(SendMessageW(hWnd(), PBM_GETPOS, 0, 0));
}

PBRANGE ProgressBar::range() const
{
	PBRANGE r{};
	SendMessageW(hWnd(), PBM_GETRANGE, 0, reinterpret_cast<LPARAM>(&r));
	return r;
}

const ProgressBar& ProgressBar::setMarquee(bool marquee) const
{
	if (marquee) {
		DWORD style = static_cast<DWORD>(GetWindowLongPtrW(hWnd(), GWL_STYLE));
		SetWindowLongPtrW(hWnd(), GWL_STYLE, style | PBS_MARQUEE); // set window style beforehand
	}

	SendMessageW(hWnd(), PBM_SETMARQUEE, marquee, 0);

	if (!marquee) {
		DWORD style = static_cast<DWORD>(GetWindowLongPtrW(hWnd(), GWL_STYLE));
		SetWindowLongPtrW(hWnd(), GWL_STYLE, style & ~PBS_MARQUEE); // remove window style
	}

	return *this;
}

const ProgressBar& ProgressBar::setPos(UINT pos) const
{
	DWORD style = static_cast<DWORD>(GetWindowLongPtrW(hWnd(), GWL_STYLE));
	if (style & PBS_MARQUEE)
		setMarquee(false); // avoid crash

	SendMessageW(hWnd(), PBM_SETPOS, pos, 0);
	return *this;	
}

const ProgressBar& ProgressBar::setRange(UINT min, UINT max) const
{
	SendMessageW(hWnd(), PBM_SETRANGE32, min, max);
	return *this;
}

const ProgressBar& ProgressBar::setState(DWORD pbst) const
{
	SendMessageW(hWnd(), PBM_SETSTATE, pbst, 0);
	return *this;
}

DWORD ProgressBar::state() const
{
	return static_cast<DWORD>(SendMessageW(hWnd(), PBM_GETSTATE, 0, 0));
}
