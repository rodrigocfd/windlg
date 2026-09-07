#include "wnd-aux.hpp"
#include "win-error.hpp"
#include <shellapi.h>
using namespace wd;

static std::pair<int, int> log_pixels() noexcept {
	static int logPixelsX = 0, logPixelsY = 0;
	if (!logPixelsX) {
		const HDC hdcScreen = GetDC(nullptr);
		logPixelsX = GetDeviceCaps(hdcScreen, LOGPIXELSX);
		logPixelsY = GetDeviceCaps(hdcScreen, LOGPIXELSY);
		ReleaseDC(nullptr, hdcScreen);
	}
	return {logPixelsX, logPixelsY};
}

int wd::dpi::x(int xVal) noexcept {
	auto [logPixelsX, logPixelsY] = log_pixels();
	return MulDiv(xVal, logPixelsX, 96);
}

int wd::dpi::y(int yVal) noexcept {
	auto [logPixelsX, logPixelsY] = log_pixels();
	return MulDiv(yVal, logPixelsY, 96);
}


Layout::Layout(const BaseContainer *pParent, size_t numCtrlsReserve)
	: _pParent{pParent}, _children{}, _szOrig{}
{
	if (numCtrlsReserve)
		_children.reserve(numCtrlsReserve);
}

Layout& Layout::add(std::initializer_list<WORD> ctrlIds, Lay layoutBehavior) {
	if (_children.empty()) {
		RECT rcParent{};
		GetClientRect(_pParent->hwnd(), &rcParent);
		_szOrig.cx = rcParent.right; // save original parent client area
		_szOrig.cy = rcParent.bottom;
	}

	_children.reserve(ctrlIds.size());

	for (auto &&ctrlId : ctrlIds) {
		const HWND hCtrl = GetDlgItem(_pParent->hwnd(), ctrlId);
		RECT rcOrig{};
		GetWindowRect(hCtrl, &rcOrig); // relative to screen
		ScreenToClient(_pParent->hwnd(), reinterpret_cast<POINT*>(&rcOrig)); // now relative to parent
		ScreenToClient(_pParent->hwnd(), reinterpret_cast<POINT*>(&rcOrig.right));
		_children.emplace_back(hCtrl, layoutBehavior, rcOrig);
	}
	return *this;
}

void Layout::rearrange(WORD req, SIZE sz) const noexcept {
	if (_children.empty() || req == SIZE_MINIMIZED)
		return; // no need to resize if window is minimized

	const HDWP hdwp = BeginDeferWindowPos(static_cast<int>(_children.size()));

	for (auto &&c : _children) {
		WORD flags = SWP_NOZORDER;

		switch (c.lay) {
		case Lay::move_move: // repos both horz and vert
			flags |= SWP_NOSIZE;
			break;
		case Lay::resz_resz: // resize both horz and vert
			flags |= SWP_NOMOVE;
		}

		DeferWindowPos(hdwp, c.hCtrl, nullptr,
			(c.lay == Lay::move_hold || c.lay == Lay::move_move || c.lay == Lay::move_resz) // horz move
				? sz.cx - _szOrig.cx + c.rcOrig.left
				: c.rcOrig.left, // keep original horz pos
			(c.lay == Lay::hold_move || c.lay == Lay::move_move || c.lay == Lay::resz_move) // vert move
				? sz.cy - _szOrig.cy + c.rcOrig.top
				: c.rcOrig.top, // keep original vert pos
			(c.lay == Lay::resz_hold || c.lay == Lay::resz_move || c.lay == Lay::resz_resz) // horz resize
				? sz.cx - _szOrig.cx + c.rcOrig.right - c.rcOrig.left
				: c.rcOrig.right - c.rcOrig.left, // keep original width
			(c.lay == Lay::hold_resz || c.lay == Lay::move_resz || c.lay == Lay::resz_resz) // vert resize
				? sz.cy - _szOrig.cy + c.rcOrig.bottom - c.rcOrig.top
				: c.rcOrig.bottom - c.rcOrig.top, // keep original height
			flags);
	}

	EndDeferWindowPos(hdwp);
}


const Menu& Menu::add(StrView text, WORD cmdId) const {
	if (!AppendMenuW(_hMenu, MF_STRING, cmdId, text.c_str())) [[unlikely]] {
		throw WinErr{GetLastError(), L"AppendMenu text failed."};
	}
	return *this;
}

const Menu& Menu::add_sep() const {
	if (!AppendMenuW(_hMenu, MF_SEPARATOR, 0, nullptr)) [[unlikely]] {
		throw WinErr{GetLastError(), L"AppendMenu sep failed."};
	}
	return *this;
}

MenuSub Menu::add_sub(StrView text) const {
	HMENU hMenuNew = CreatePopupMenu();
	if (!hMenuNew) [[unlikely]] {
		throw WinErr{GetLastError(), L"CreatePopupMenu failed."};
	}
	AppendMenuW(_hMenu, MF_POPUP, reinterpret_cast<UINT_PTR>(hMenuNew), text.c_str()); // now owned by parent menu
	return MenuSub{_hMenu, hMenuNew, nullptr}; // non-owning
}

const Menu& Menu::enable_cmd(bool doEnable, WORD cmdId) const noexcept {
	EnableMenuItem(_hMenu, cmdId, MF_BYCOMMAND | (doEnable ? MF_ENABLED : MF_GRAYED));
	return *this;
}

const Menu& Menu::enable_cmds(bool doEnable, std::initializer_list<WORD> cmdIds) const noexcept {
	for (auto &&cmdId : cmdIds)
		enable_cmd(doEnable, cmdId);
	return *this;
}

const Menu& Menu::set_default_cmd(WORD cmdId) const {
	if (!SetMenuDefaultItem(_hMenu, cmdId, FALSE)) [[unlikely]] {
		throw WinErr{GetLastError(), L"SetMenuDefaultItem failed."};
	}
	return *this;
}

const Menu& Menu::set_cmd_text(WORD cmdId, StrView text) const {
	const MENUITEMINFOW mii{
		.cbSize = sizeof(MENUITEMINFOW),
		.fMask = MIIM_STRING,
		.dwTypeData = const_cast<LPWSTR>(text.c_str()),
	};
	if (!SetMenuItemInfoW(_hMenu, cmdId, FALSE, &mii)) [[unlikely]] {
		throw WinErr{GetLastError(), L"SetMenuItemInfo failed."};
	}
	return *this;
}

const Menu& Menu::show_at_point(POINT pos, HWND hOwner, HWND hChildCoordsRelativeTo) const {
	ClientToScreen(hChildCoordsRelativeTo, &pos);
	SetForegroundWindow(hOwner);
	if (!TrackPopupMenu(_hMenu, TPM_LEFTBUTTON, pos.x, pos.y, 0, hOwner, nullptr)) [[unlikely]] {
		throw WinErr{GetLastError(), L"TrackPopupMenu failed."};
	}
	PostMessageW(hOwner, WM_NULL, 0, 0); // necessary according to TrackPopupMenu docs
	return *this;
}


MenuResource& MenuResource::operator=(MenuResource &&other) noexcept {
	destroy();
	std::swap(_hMenu, other._hMenu);
	return *this;
}

MenuResource& MenuResource::destroy() noexcept {
	if (_hMenu) {
		DestroyMenu(_hMenu);
		_hMenu = nullptr;
	}
	return *this;
}

MenuResource& MenuResource::create_popup() {
	destroy();
	_hMenu = CreatePopupMenu();
	if (!_hMenu) [[unlikely]] {
		throw WinErr{GetLastError(), L"CreatePopupMenu failed."};
	}
	return *this;
}

MenuResource& MenuResource::create_toplevel() {
	destroy();
	_hMenu = CreateMenu();
	if (!_hMenu) [[unlikely]] {
		throw WinErr{GetLastError(), L"CreateMenu failed."};
	}
	return *this;
}

HMENU MenuResource::leak() noexcept {
	HMENU h = _hMenu;
	_hMenu = nullptr;
	return h;
}

MenuResource& MenuResource::load_from_resource(HINSTANCE hInst, WORD menuId) {
	destroy();
	_hMenu = LoadMenuW(hInst, MAKEINTRESOURCEW(menuId));
	if (!_hMenu) [[unlikely]] {
		throw WinErr{GetLastError(), L"LoadMenu failed."};
	}
	return *this;
}

MenuResource& MenuResource::load_from_resource(HWND hParent, WORD menuId) {
	return load_from_resource(reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(hParent, GWLP_HINSTANCE)), menuId);
}


const MenuSub& MenuSub::add(StrView text, WORD cmdId) const {
	_current().add(text, cmdId);
	return *this;
}

const MenuSub& MenuSub::add_sep() const {
	_current().add_sep();
	return *this;
}

MenuSub MenuSub::add_sub(StrView text) const {
	if (_hMenu2) [[unlikely]] {
		throw WinErr{ERROR_BAD_ARGUMENTS, L"MenuSub must be at max 3 levels deep. If you need more, create it manually."};
	}

	HMENU hMenuNew = CreatePopupMenu();
	if (!hMenuNew) [[unlikely]] {
		throw WinErr{GetLastError(), L"CreatePopupMenu failed."};
	}
	AppendMenuW(_current().hmenu(), MF_POPUP, reinterpret_cast<UINT_PTR>(hMenuNew), text.c_str()); // now owned by parent menu

	return _hMenu1 ? MenuSub{_hMenu0, _hMenu1, hMenuNew} : MenuSub{_hMenu0, hMenuNew, nullptr};
}

Menu MenuSub::_current() const noexcept {
	if (_hMenu2)      return Menu{_hMenu2};
	else if (_hMenu1) return Menu{_hMenu1};
	else              return Menu{_hMenu0};
}


const ImageList& ImageList::add_file_ext(std::initializer_list<wd::StrView> fileExts) const {
	SHFILEINFOW shfi{};
	std::wstring extBuf{};

	for (auto &&fileExt : fileExts) {
		shfi = {0};
		extBuf = L"*.";
		extBuf.append(fileExt.c_str());
		const DWORD_PTR ok = SHGetFileInfoW(extBuf.c_str(), FILE_ATTRIBUTE_NORMAL, &shfi, sizeof(shfi),
			SHGFI_USEFILEATTRIBUTES | SHGFI_ICON | (_resolution == 16 ? SHGFI_SMALLICON : SHGFI_LARGEICON));
		if (!ok) [[unlikely]] {
			throw WinErr{ERROR_UNIDENTIFIED_ERROR, L"SHGetFileInfo failed."};
		}

#ifdef _MSC_VER
#pragma warning(disable: 6387)
#endif
		ImageList_AddIcon(_hIL, shfi.hIcon);
		DestroyIcon(shfi.hIcon);
	}
	return *this;
}

const ImageList& ImageList::add_resource(std::initializer_list<WORD> iconIds) const {
	const HINSTANCE hInst = GetModuleHandleW(nullptr);
	for (auto &&iconId : iconIds) {
		const HICON hIcon = LoadIconW(hInst, MAKEINTRESOURCEW(iconId));
		if (!hIcon) [[unlikely]] {
			throw WinErr{GetLastError(), L"LoadIcon failed."};
		}

		ImageList_AddIcon(_hIL, hIcon);
	}
	return *this;
}

ImageList& ImageList::destroy() noexcept {
	if (_hIL) {
		ImageList_Destroy(_hIL);
		_hIL = nullptr;
	}
	return *this;
}

ImageList& ImageList::_raw_create(int resolution) {
	destroy();
	_hIL = ImageList_Create(resolution, resolution, ILC_COLOR32, 1, 4); // arbitrary grow size
	if (!_hIL) [[unlikely]] {
		throw WinErr{ERROR_UNIDENTIFIED_ERROR, L"ImageList_Create failed."};
	}

	_resolution = resolution;
	return *this;
}
