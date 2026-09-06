#include "wnd-ctl.hpp"
#include "win-error.hpp"
using namespace wd;

void wd::enable_many(const BaseContainer *pParent, bool doEnable, std::initializer_list<WORD> ctrlIds) noexcept {
	for (auto &&ctrlId : ctrlIds)
		EnableWindow(GetDlgItem(pParent->hwnd(), ctrlId), doEnable);
}

void wd::enable_many(const BaseContainer *pParent, bool doEnable, std::span<const WORD> ctrlIds) noexcept {
	for (auto &&ctrlId : ctrlIds)
		EnableWindow(GetDlgItem(pParent->hwnd(), ctrlId), doEnable);
}


const BaseNativeCtrl& BaseNativeCtrl::enable(bool doEnable) const noexcept {
	EnableWindow(hwnd(), doEnable);
	return *this;
}

const BaseNativeCtrl& BaseNativeCtrl::focus() const noexcept {
	PostMessageW(GetParent(hwnd()), WM_NEXTDLGCTL, reinterpret_cast<WPARAM>(hwnd()), MAKELPARAM(TRUE, 0));
	return *this;
}


std::wstring Button::text() const {
	return _wd_internal::get_wnd_text(hwnd());
}

const Button& Button::set_text(StrView text) const noexcept {
	SetWindowTextW(hwnd(), text.c_str());
	return *this;
}


bool CheckBox::is_checked() const noexcept {
	return SendMessageW(hwnd(), BM_GETCHECK, 0, 0) == BST_CHECKED;
}

const CheckBox& CheckBox::set_check(bool doCheck) const noexcept {
	SendMessageW(hwnd(), BM_SETCHECK, doCheck ? BST_CHECKED : BST_UNCHECKED, 0);
	return *this;
}

const CheckBox& CheckBox::set_check_and_trigger(bool doCheck) const noexcept {
	set_check(doCheck);
	SendMessageW(parent()->hwnd(), WM_COMMAND, MAKEWPARAM(ctrl_id(), BN_CLICKED), reinterpret_cast<LPARAM>(hwnd()));
	return *this;
}

std::wstring CheckBox::text() const {
	return _wd_internal::get_wnd_text(hwnd());
}

const CheckBox& CheckBox::set_text(StrView text) const noexcept {
	SetWindowTextW(hwnd(), text.c_str());
	return *this;
}

const CheckBox& CheckBox::set_text_and_resize(StrView text) const {
	set_text(text);
	SIZE sz{};
	[[maybe_unused]] LRESULT ok = SendMessageW(hwnd(), BCM_GETIDEALSIZE, 0, reinterpret_cast<LPARAM>(&sz));
#ifdef _DEBUG
	if (!ok)
		throw WinErr{ERROR_UNIDENTIFIED_ERROR, L"BCM_GETIDEALSIZE failed, do you have an app manifest?"};
#endif
	SetWindowPos(hwnd(), nullptr, 0, 0, sz.cx, sz.cy, SWP_NOZORDER | SWP_NOMOVE);
	return *this;
}


const ComboBox::Item& ComboBox::Item::del() const noexcept {
	SendMessageW(combo_box().hwnd(), CB_DELETESTRING, _index, 0);
	return *this;
}

const ComboBox::Item& ComboBox::Item::select() const noexcept {
	SendMessageW(combo_box().hwnd(), CB_SETCURSEL, _index, 0);
	return *this;
}

const ComboBox::Item& ComboBox::Item::set_text(StrView text) const noexcept {
	HWND hCmb = combo_box().hwnd();
	SendMessageW(hCmb, CB_DELETESTRING, _index, 0);
	SendMessageW(hCmb, CB_INSERTSTRING, _index, reinterpret_cast<LPARAM>(text.c_str()));
	return *this;
}

std::wstring ComboBox::Item::text() const {
	HWND hCmb = combo_box().hwnd();
	const size_t nChars = SendMessageW(hCmb, CB_GETLBTEXTLEN, _index, 0);
	std::wstring s(nChars + 1, L'\0');
	SendMessageW(hCmb, CB_GETLBTEXT, _index, reinterpret_cast<LPARAM>(s.data()));
	s.resize(nChars);
	return s;
}


ComboBox::Item ComboBox::item_add(StrView text) const noexcept {
	LRESULT idx = SendMessageW(hwnd(), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(text.c_str()));
	return Item{*this, static_cast<int>(idx)};
}

const ComboBox& ComboBox::item_add(std::initializer_list<StrView> texts) const noexcept {
	for (auto &&text : texts)
		item_add(text);
	return *this;
}

const ComboBox& ComboBox::item_add(std::span<const wchar_t* const> texts) const noexcept {
	// 1st const - the characters are immutable.
	// 2nd const - the pointers in the span are immutable.
	for (auto &&text : texts)
		item_add(text);
	return *this;
}

const ComboBox& ComboBox::item_add(std::span<const StrView> texts) const noexcept {
	for (auto &&text : texts)
		item_add(text);
	return *this;
}

int ComboBox::item_count() const noexcept {
	return static_cast<int>(SendMessageW(hwnd(), CB_GETCOUNT, 0, 0));
}

const ComboBox& ComboBox::item_del_all() const noexcept {
	SendMessageW(hwnd(), CB_RESETCONTENT, 0, 0);
	return *this;
}

ComboBox::Item ComboBox::item_selected() const noexcept {
	return Item{*this, static_cast<int>(SendMessageW(hwnd(), CB_GETCURSEL, 0, 0))};
}

std::wstring ComboBox::text() const {
	return _wd_internal::get_wnd_text(hwnd());
}


SYSTEMTIME DateTimePicker::time() const noexcept {
	SYSTEMTIME st{};
	DateTime_GetSystemtime(hwnd(), &st);
	return st;
}

const DateTimePicker& DateTimePicker::set_time(const SYSTEMTIME &st) const noexcept {
	DateTime_SetSystemtime(hwnd(), GDT_VALID, &st);
	return *this;
}

const DateTimePicker& DateTimePicker::set_time(const FILETIME &ft) const noexcept {
	SYSTEMTIME st{};
	FileTimeToSystemTime(&ft, &st);
	return set_time(st);
}


int ProgressBar::pos() const noexcept {
	return static_cast<int>(SendMessageW(hwnd(), PBM_GETPOS, 0, 0));
}

PBRANGE ProgressBar::range() const noexcept {
	PBRANGE pbr{};
	SendMessageW(hwnd(), PBM_GETRANGE, 0, reinterpret_cast<LPARAM>(&pbr));
	return pbr;
}

ProgressBar& ProgressBar::set_marquee(bool isMarquee) noexcept {
	const DWORD curStyle = static_cast<DWORD>(GetWindowLongPtrW(hwnd(), GWL_STYLE));
	if (isMarquee)
		SetWindowLongPtrW(hwnd(), GWL_STYLE, curStyle | PBS_MARQUEE);

	SendMessageW(hwnd(), PBM_SETMARQUEE, isMarquee ? TRUE : FALSE, 0);

	if (!isMarquee)
		SetWindowLongPtrW(hwnd(), GWL_STYLE, curStyle & ~PBS_MARQUEE);

	_isMarquee = isMarquee;
	return *this;
}

ProgressBar& ProgressBar::set_pos(int newPos) noexcept {
	if (_isMarquee)
		set_marquee(true); // avoid crash

	SendMessageW(hwnd(), PBM_SETPOS, newPos, 0);
	return *this;
}

const ProgressBar& ProgressBar::set_range(int minVal, int maxVal) const noexcept {
	SendMessageW(hwnd(), PBM_SETRANGE32, minVal, maxVal);
	return *this;
}

const ProgressBar& ProgressBar::set_state(State state) const noexcept {
	SendMessageW(hwnd(), PBM_SETSTATE, static_cast<LPARAM>(state), 0);
	return *this;
}


std::wstring Static::text() const {
	return _wd_internal::get_wnd_text(hwnd());
}

const Static& Static::set_text(StrView text) const noexcept {
	SetWindowTextW(hwnd(), text.c_str());
	return *this;
}

const Static& Static::set_text_and_resize(StrView text) const {
	set_text(text); // TODO: remove accel ampersands

	const HWND hwndDesktop = GetDesktopWindow();
	const HDC hdcDesktop = GetDC(hwndDesktop);
	const HDC hdcClone = CreateCompatibleDC(hdcDesktop);

	NONCLIENTMETRICSW ncm{.cbSize = sizeof(NONCLIENTMETRICSW)};
	SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
	const HFONT hSysFont = CreateFontIndirectW(&ncm.lfMenuFont);
	if (!hSysFont) [[unlikely]] {
		throw WinErr{ERROR_UNIDENTIFIED_ERROR, L"CreateFontIndirect failed."};
	}
	const HFONT hPrevFont = reinterpret_cast<HFONT>(SelectObject(hdcClone, hSysFont));

	std::wstring text2{text.c_str(), text.length()};
	str::trim_spaces(text2);
	const bool isTextEmpty = text2.empty();
	text2 = isTextEmpty ? L"Pj" : std::wstring{text.c_str(), text.length()}; // placeholder to get height

	SIZE bounds{};
	GetTextExtentPoint32W(hdcClone, text2.c_str(), static_cast<int>(text2.length()), &bounds);
	if (isTextEmpty)
		bounds.cx = 0; // if no text was given, return just the height

	if (text2.find_first_of(L'\n') != std::wstring::npos) { // multi-line, height must be recalculated
		RECT rc{};
		DrawTextW(hdcClone, text2.c_str(), static_cast<int>(text2.length()), &rc, DT_CALCRECT | DT_EDITCONTROL);
		bounds.cx = rc.right;
		bounds.cy = rc.bottom;
	}

	SetWindowPos(hwnd(), nullptr, 0, 0, bounds.cx, bounds.cy, SWP_NOZORDER | SWP_NOMOVE);

	SelectObject(hdcClone, hPrevFont);
	DeleteObject(hSysFont);
	DeleteDC(hdcClone);
	ReleaseDC(hwndDesktop, hdcDesktop);
	return *this;
}


const StatusBar::Part& StatusBar::Part::set_text(StrView text) const noexcept {
	SendMessageW(status_bar().hwnd(), SB_SETTEXTW,
		MAKEWPARAM(MAKEWORD(_index, 0), 0),
		reinterpret_cast<LPARAM>(text.c_str()));
	return *this;
}

std::wstring StatusBar::Part::text() const {
	const HWND hSb = status_bar().hwnd();
	const WORD len = LOWORD(static_cast<DWORD>(SendMessageW(hSb, SB_GETTEXTLENGTHW, _index, 0)));

	std::wstring buf(len + 1, L'\0');
	SendMessageW(hSb, SB_GETTEXTW, _index, reinterpret_cast<LPARAM>(buf.data()));
	buf.resize(len);
	return buf;
}


StatusBar::StatusBar(const BaseContainer *pParent, WORD ctrlId) noexcept
	: BaseNativeCtrl{pParent, (ctrlId > 0) ? ctrlId : _wd_internal::next_auto_ctrl_id()}
{
}

StatusBar& StatusBar::resize_to_parent(WPARAM wp, LPARAM lp) noexcept {
	if (wp == SIZE_MINIMIZED || !hwnd())
		return *this;

	SendMessageW(hwnd(), WM_SIZE, 0, 0); // tell status bar to fit parent
	if (_partsData.empty())
		return *this;

	int totalWeight = 0; // total weight of all flex parts
	int cxVariable = LOWORD(lp); // remaning width to be divided among flex parts
	for (size_t i = 0; i < part_count(); ++i) {
		if (_part_is_fixed(i))
			cxVariable -= _partsData[i].sizePixels; // decrease available width room
		else
			totalWeight += _partsData[i].resizeWeight;
	}

	int cxTotal = LOWORD(lp);
	for (size_t i = part_count(); i-- > 0; ) { // fill right edges array with the right edge of each part
		_rightEdges[i] = cxTotal;
		if (_part_is_fixed(i))
			cxTotal -= _partsData[i].sizePixels;
		else
			cxTotal -= (cxVariable / totalWeight) * _partsData[i].resizeWeight;
	}
	SendMessageW(hwnd(), SB_SETPARTS, _rightEdges.size(), reinterpret_cast<LPARAM>(_rightEdges.data()));

	return *this;
}

StatusBar& StatusBar::_raw_add_part(int sizePixels, int resizeWeight) {
	if (!hwnd()) { // if not created yet
		const HINSTANCE hInst = reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(parent()->hwnd(), GWLP_HINSTANCE));
		const DWORD parentStyle = static_cast<DWORD>(GetWindowLongPtrW(parent()->hwnd(), GWL_STYLE));
		const bool isParentResizable = (parentStyle & WS_MAXIMIZEBOX) || (parentStyle & WS_SIZEBOX);
		const DWORD style = WS_CHILD | WS_VISIBLE | SBARS_TOOLTIPS | (isParentResizable ? SBARS_SIZEGRIP : 0);

		CreateWindowExW(WS_EX_LEFT, STATUSCLASSNAMEW, nullptr, style, 0, 0, 0, 0,
			parent()->hwnd(), reinterpret_cast<HMENU>(ctrl_id()), hInst, nullptr);
		if (!hwnd()) [[unlikely]] {
			throw WinErr{GetLastError(), L"CreateWindowEx failed."};
		}
	}

	_partsData.emplace_back(sizePixels, resizeWeight);
	_rightEdges.emplace_back(0);

	RECT rcParent{};
	GetClientRect(parent()->hwnd(), &rcParent);
	resize_to_parent(SIZE_RESTORED, MAKELPARAM(rcParent.right, 0)); // will create all parts
	return *this;
}
