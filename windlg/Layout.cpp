#include "Layout.h"
using namespace lib;

Layout& Layout::add(Act horz, Act vert, std::initializer_list<WORD> ctrlIds)
{
	if (horz != Act::None || vert != Act::None) { // if nothing to do, don't even bother adding it
		for (auto&& ctrlId : ctrlIds)
			_ctrls.emplace_back(ctrlId, horz, vert, std::nullopt);
	}
	return *this;
}

void Layout::autoArrange(Dialog* parent, UINT uMsg, WPARAM wp, LPARAM lp)
{
	if (uMsg == WM_INITDIALOG) {
		_hParent = parent->hWnd(); // initialize the Layout, from now on WM_SIZE will be processed
		RECT rc{};
		GetClientRect(_hParent, &rc);
		_szParentOrig = {.cx = rc.right, .cy = rc.bottom};
	} else if (_hParent && uMsg == WM_SIZE && !_ctrls.empty() && wp != SIZE_MINIMIZED) {
		_rearrangeCtrls(LOWORD(lp), HIWORD(lp));
	}
}

void Layout::_rearrangeCtrls(UINT cxParent, UINT cyParent)
{
	HDWP hdwp = BeginDeferWindowPos(static_cast<int>(_ctrls.size()));
	for (auto& ctrl : _ctrls) {
		UINT swp = SWP_NOZORDER;
		HWND hCtrl = GetDlgItem(_hParent, ctrl.ctrlId);

		if (ctrl.horz == Act::Repos && ctrl.vert == Act::Repos) { // reposition both horz & vert
			swp |= SWP_NOSIZE;
		} else if (ctrl.horz == Act::Resize && ctrl.vert == Act::Resize) { // resize both horz & vert
			swp |= SWP_NOMOVE;
		}

		if (!ctrl.rcOrig.has_value()) { // first pass: original child client coords not saved yet
			RECT rc{};
			GetWindowRect(hCtrl, &rc);
			ScreenToClient(_hParent, reinterpret_cast<POINT*>(&rc));
			ScreenToClient(_hParent, reinterpret_cast<POINT*>(&rc.right));
			ctrl.rcOrig = {rc};
		}
		RECT rcOrig = ctrl.rcOrig.value();

		DeferWindowPos(hdwp, hCtrl, nullptr,
			ctrl.horz == Act::Repos ? cxParent - _szParentOrig.cx + rcOrig.left
			: rcOrig.left,
			ctrl.vert == Act::Repos ? cyParent - _szParentOrig.cy + rcOrig.top
			: rcOrig.top,
			ctrl.horz == Act::Resize ? cxParent - _szParentOrig.cx + rcOrig.right - rcOrig.left
			: rcOrig.right - rcOrig.left,
			ctrl.vert == Act::Resize ? cyParent - _szParentOrig.cy + rcOrig.bottom - rcOrig.top
			: rcOrig.bottom - rcOrig.top,
			swp);
	}
	EndDeferWindowPos(hdwp);
}
