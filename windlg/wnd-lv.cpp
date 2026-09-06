#include "wnd-lv.hpp"
#include "win-error.hpp"
#include "wnd-aux.hpp"
using namespace wd;

std::vector<std::wstring> ListView::Col::selected_texts() const {
	const ListView lv{list_view()};
	const HWND hLv = lv.hwnd();
	const int numSel = lv.item_selected_count();
	std::vector<std::wstring> texts{};
	texts.reserve(numSel);

	int idx = -1;
	for (;;) {
		idx = ListView_GetNextItem(hLv, idx, LVNI_ALL | LVNI_SELECTED);
		if (idx == -1) break;
		texts.emplace_back(Item{lv, idx}.text(_index));
	}
	return texts;
}

const ListView::Col& ListView::Col::set_align(Align align) const noexcept {
	const HWND hHeader = ListView_GetHeader(list_view().hwnd());
	HDITEMW hdi{.mask = HDI_FORMAT};
	Header_GetItem(hHeader, _index, &hdi); // first, retrieve current

	hdi.fmt &= ~(HDF_CENTER | HDF_LEFT | HDF_RIGHT); // clear all three
	hdi.fmt |= (static_cast<WORD>(align) & (HDF_LEFT | HDF_CENTER | HDF_RIGHT)); // sanitize
	Header_SetItem(hHeader, _index, &hdi);

	return *this;
}

const ListView::Col& ListView::Col::set_arrow(Arrow arrow) const noexcept {
	const HWND hHeader = ListView_GetHeader(list_view().hwnd());
	const int numCols = Header_GetItemCount(hHeader);

	for (int i = 0; i < numCols; ++i) {
		HDITEMW hdi{.mask = HDI_FORMAT};
		Header_GetItem(hHeader, i, &hdi); // first, retrieve current

		hdi.fmt &= ~(HDF_SORTDOWN | HDF_SORTUP); // clear all two
		if (i == _index) // only the targeted column will have the flag set
			hdi.fmt |= (static_cast<WORD>(arrow) & (HDF_SORTDOWN | HDF_SORTUP)); // sanitize
		Header_SetItem(hHeader, i, &hdi);
	}

	return *this;
}

const ListView::Col& ListView::Col::set_width(UINT width) const noexcept {
	ListView_SetColumnWidth(list_view().hwnd(), _index, width);
	return *this;
}

const ListView::Col& ListView::Col::set_width_to_fill() const noexcept {
	const HWND hLv = list_view().hwnd();
	const int numCols = Header_GetItemCount(ListView_GetHeader(hLv));
	if (numCols == 0)
		return *this;

	UINT cxUsed = 0;
	for (int i = 0; i < numCols; ++i) {
		if (i != _index)
			cxUsed += ListView_GetColumnWidth(hLv, i); // retrieve cx of each column, but us
	}

	RECT rc{};
	GetClientRect(hLv, &rc); // list view client area
	return set_width(rc.right - cxUsed);
}

std::vector<std::wstring> ListView::Col::texts() const {
	const ListView lv{list_view()};
	const int numItems = lv.item_count();
	std::vector<std::wstring> texts{};
	texts.reserve(numItems);
	for (int i = 0; i < numItems; ++i)
		texts.emplace_back(Item{lv, i}.text(_index));
	return texts;
}

UINT ListView::Col::width() const noexcept {
	return ListView_GetColumnWidth(list_view().hwnd(), _index);
}


const ListView::Item& ListView::Item::del() const noexcept {
	ListView_DeleteItem(list_view().hwnd(), _index);
	return *this;
}

const ListView::Item& ListView::Item::focus() const noexcept {
	ListView_SetItemState(list_view().hwnd(), _index, LVIS_FOCUSED, LVIS_FOCUSED);
	return *this;
}

int ListView::Item::icon() const noexcept {
	LVITEMW lvi{
		.mask = LVIF_IMAGE,
		.iItem = _index,
	};
	ListView_GetItem(list_view().hwnd(), &lvi);
	return lvi.iImage;
}

bool ListView::Item::is_focused() const noexcept {
	return ListView_GetItemState(list_view().hwnd(), _index, LVIS_FOCUSED) == LVIS_FOCUSED;
}

bool ListView::Item::is_selected() const noexcept {
	return ListView_GetItemState(list_view().hwnd(), _index, LVIS_SELECTED) == LVIS_SELECTED;
}

bool ListView::Item::is_visible() const noexcept {
	return ListView_IsItemVisible(list_view().hwnd(), _index);
}

const ListView::Item& ListView::Item::select(bool doSelect) const noexcept {
	ListView_SetItemState(list_view().hwnd(), _index, doSelect ? LVIS_SELECTED : 0, LVIS_SELECTED);
	return *this;
}

const ListView::Item& ListView::Item::set_icon(int iconIndex) const noexcept {
	LVITEMW lvi{
		.mask = LVIF_IMAGE,
		.iItem = _index,
		.iImage = iconIndex,
	};
	ListView_SetItem(list_view().hwnd(), &lvi);
	return *this;
}

const ListView::Item& ListView::Item::set_text(int col, StrView text) const noexcept {
	ListView_SetItemText(list_view().hwnd(), _index, col, const_cast<LPWSTR>(text.c_str()));
	return *this;
}

const ListView::Item& ListView::Item::set_texts(std::initializer_list<StrView> texts) const noexcept {
	int i = 0;
	for (auto &&t : texts)
		set_text(i++, t);
	return *this;
}

std::wstring ListView::Item::text(int col) const {
	const HWND hLv = list_view().hwnd();
	UINT curBufSz = static_cast<UINT>(std::wstring{}.capacity());
	std::wstring buf{};

	for (;;) {
		buf.resize(curBufSz);

		LVITEMW lvi{
			.mask = LVIF_TEXT,
			.iSubItem = static_cast<int>(col),
			.pszText = buf.data(),
			.cchTextMax = static_cast<int>(curBufSz),
		};

		UINT recvChars = static_cast<UINT>(
			SendMessageW(hLv, LVM_GETITEMTEXTW, _index, reinterpret_cast<LPARAM>(&lvi)));
		recvChars += 1; // plus terminating null count

		if (recvChars < curBufSz) { // to break, must have at least 1 char gap
			buf.resize(recvChars - 1);
			return buf;
		}

		curBufSz *= 2; // double the buffer size to try again
	}
}

LPARAM ListView::Item::_raw_data() const noexcept {
	LVITEMW lvi{
		.mask = LVIF_PARAM,
		.iItem = _index,
	};
	ListView_GetItem(list_view().hwnd(), &lvi);
	return lvi.lParam;
}

const ListView::Item& ListView::Item::_raw_set_data(LPARAM value) const noexcept {
	LVITEMW lvi{
		.mask = LVIF_PARAM,
		.iItem = _index,
		.lParam = value,
	};
	ListView_SetItem(list_view().hwnd(), &lvi);
	return *this;
}


static int raw_add_col(HWND hLv, StrView text, UINT width) noexcept {
	LVCOLUMNW lvc{
		.mask = LVCF_TEXT | LVCF_WIDTH,
		.cx = static_cast<int>(width),
		.pszText = const_cast<LPWSTR>(text.c_str()),
	};
	return ListView_InsertColumn(hLv, 0xffff, &lvc); // insert as the last column
}

const ListView& ListView::col_add(StrView text, UINT width) const noexcept {
	raw_add_col(hwnd(), text, width);
	return *this;
}

const ListView& ListView::col_add(StrView text, UINT width, Align align) const noexcept {
	int idx = raw_add_col(hwnd(), text, width);
	if (align != Align::left)
		Col{*this, idx}.set_align(align);
	return *this;
}

int ListView::col_count() const noexcept {
	return Header_GetItemCount(ListView_GetHeader(hwnd()));
}


ListView::Item ListView::item_add(std::initializer_list<StrView> texts, int iconIndex) const {
	LVITEMW lvi{
		.mask = LVIF_TEXT | static_cast<UINT>(iconIndex > -1 ? LVIF_IMAGE : 0),
		.iItem = 0x0fff'ffff, // insert as the last item
		.pszText = const_cast<LPWSTR>(texts.begin()->c_str()),
		.iImage = iconIndex,
	};
	const int newIdx = ListView_InsertItem(hwnd(), &lvi);
#ifdef _DEBUG
	if (newIdx == -1)
		throw WinErr{ERROR_UNIDENTIFIED_ERROR, L"ListView_InsertItem failed."};
#endif
	const Item newItem = Item{*this, newIdx};

	for (int i = 1; i < static_cast<int>(texts.size()); ++i) // set subsequent cols
		newItem.set_text(i, (texts.begin() + i)->c_str());

	return newItem;
}

int ListView::item_count() const noexcept {
	return ListView_GetItemCount(hwnd());
}

const ListView& ListView::item_del_all() const noexcept {
	ListView_DeleteAllItems(hwnd());
	return *this;
}

const ListView& ListView::item_del_selected() const noexcept {
	for (;;) {
		const int idxFound = ListView_GetNextItem(hwnd(), -1, LVNI_ALL | LVNI_SELECTED); // always search first one
		if (idxFound == -1) break;
		Item{*this, idxFound}.del();
	}
	return *this;
}

ListView::Item ListView::item_focused() const noexcept {
	return Item{*this, ListView_GetNextItem(hwnd(), -1, LVNI_ALL | LVNI_FOCUSED)};
}

const ListView& ListView::item_select_all(bool doSelect) const noexcept {
	const HWND hLv = hwnd();
	const bool isSingleSel = GetWindowLongPtrW(hLv, GWL_STYLE) & LVS_SINGLESEL;
	if (!isSingleSel) [[likely]] {
		ListView_SetItemState(hLv, -1, doSelect ? LVIS_SELECTED : 0, LVIS_SELECTED);
	}
	return *this;
}

std::vector<ListView::Item> ListView::item_selected() const {
	const HWND hLv = hwnd();
	std::vector<Item> items{};
	items.reserve(item_selected_count());

	int idx = -1;
	for (;;) {
		idx = ListView_GetNextItem(hLv, idx, LVNI_ALL | LVNI_SELECTED);
		if (idx == -1) break;
		items.emplace_back(*this, idx);
	}
	return items;
}

int ListView::item_selected_count() const noexcept {
	return ListView_GetSelectedCount(hwnd());
}


const ListView& ListView::sort(std::initializer_list<SortCriterion> criteria) const {
	if (!criteria.size()) [[unlikely]] {
		throw WinErr{ERROR_BAD_ARGUMENTS, L"No sort criteria specified."};
	}

	sort([this, criteria](Item a, Item b) -> int {
		for (auto &&cond : criteria) {
			int cmp = 0;

			switch (cond.sort) {
			case Sort::asc:
				cmp = a.text(cond.index).compare(b.text(cond.index));
				break;
			case Sort::desc:
				cmp = b.text(cond.index).compare(a.text(cond.index));
				break;
			case Sort::asc_i:
				cmp = str::compare_i(a.text(cond.index), b.text(cond.index));
				break;
			case Sort::desc_i:
				cmp = str::compare_i(b.text(cond.index), a.text(cond.index));
			}

			if (cmp)
				return cmp; // no need to compare next col
		}
		return 0; // all cols equal for the 2 items being compared
	});

	const SortCriterion crit0 = *criteria.begin();
	const Arrow arrow = (crit0.sort == Sort::asc || crit0.sort == Sort::asc_i) // 1st item will dictate it
		? Arrow::asc : Arrow::desc;
	Col{*this, crit0.index}.set_arrow(arrow); // courtesy: set the col arrow
	return *this;
}

const ListView& ListView::sort(std::function<int(Item a, Item b)> cb) const {
	struct Info final {
		const ListView &lv;
		std::function<int(Item, Item)> cb;
	};
	Info nfo{.lv = *this, .cb = std::move(cb)};

	ListView_SortItemsEx(hwnd(), [](LPARAM idxA, LPARAM idxB, LPARAM lp) -> int { // receives indexes
		auto pNfo = reinterpret_cast<Info*>(lp);
		return pNfo->cb( Item{pNfo->lv, static_cast<int>(idxA)}, Item{pNfo->lv, static_cast<int>(idxB)} );
	}, &nfo);

	return *this;
}

static void lv_context_menu(HWND hLv, HMENU hMenu, POINT coords, bool hasCtrl, bool hasShift) {
	const HWND hParent = GetParent(hLv);
	if (coords.x != -1 && coords.y != -1) { // fired by a right-click
		ScreenToClient(hLv, &coords); // now relative to listview
		LVHITTESTINFO hti{.pt = coords};
		const int idxOver = ListView_HitTest(hLv, &hti);
		if (idxOver == -1) { // no item was right-clicked
			ListView_SetItemState(hLv, -1, 0, LVIS_SELECTED); // deselect all
		} else if (!hasCtrl && !hasShift) {
			ListView_SetItemState(hLv, idxOver, LVIS_SELECTED | LVIS_FOCUSED,
				LVIS_SELECTED | LVIS_FOCUSED); // if not yet
		}
		PostMessageW(hParent, WM_NEXTDLGCTL, // because a right-click won't set the focus by itself
			reinterpret_cast<WPARAM>(hLv), MAKELPARAM(TRUE, 0));
	} else { // fired by the context meny key or Shift+F10
		const int idxFocus = ListView_GetNextItem(hLv, -1, LVNI_ALL | LVNI_FOCUSED);
		if (idxFocus != -1 && ListView_IsItemVisible(hLv, idxFocus)) {
			RECT rc{};
			ListView_GetItemRect(hLv, idxFocus, &rc, LVIR_BOUNDS);
			coords = {.x = rc.left + 16, .y = rc.top + (rc.bottom - rc.top) / 2};
		} else { // no item is focused and visible
			coords = {.x = 6, .y = 10}; // arbitrary coordinates
		}
	}

	Menu{hMenu}.show_at_point(coords, hParent, hLv);
}

static LRESULT CALLBACK lv_subclass_proc(HWND hLv, UINT uMsg,
	WPARAM wp, LPARAM lp, UINT_PTR idSubclass, DWORD_PTR refData)
{
	HMENU hMenu = reinterpret_cast<HMENU>(refData);

	if (uMsg == WM_KEYDOWN && wp == 'A') {
		const bool hasCtrl = GetKeyState(VK_CONTROL) & 0x8000;
		if (hasCtrl) {
			ListView_SetItemState(hLv, -1, LVIS_SELECTED, LVIS_SELECTED); // Ctrl+A will select all items
		}
	} else if (uMsg == WM_CONTEXTMENU && hMenu) { // right click, context menu key or Shift+F10
		const bool hasCtrl = GetKeyState(VK_CONTROL) & 0x8000;
		const bool hasShift = GetKeyState(VK_SHIFT) & 0x8000;
		const POINT screenCoords{
			.x = static_cast<short>(LOWORD(lp)),
			.y = static_cast<short>(HIWORD(lp)),
		};
		lv_context_menu(hLv, hMenu, screenCoords, hasCtrl, hasShift);
	} else if (uMsg == WM_GETDLGCODE) {
		if (lp && wp == VK_RETURN) { // Enter key
			const WORD ctrlId = static_cast<WORD>(GetDlgCtrlID(hLv));
			NMLVKEYDOWN lvkd{
				.hdr{
					.hwndFrom = hLv,
					.idFrom = ctrlId,
					.code = LVN_KEYDOWN,
			},
			.wVKey = VK_RETURN,
			};
			SendMessageW(GetAncestor(hLv, GA_PARENT), WM_NOTIFY, ctrlId,
				reinterpret_cast<LPARAM>(&lvkd)); // forward to parent
			return DefSubclassProc(hLv, WM_GETDLGCODE, wp, lp); // dlgcSystem
		}
	} else if (uMsg == WM_NOTIFY) {
		auto pHdr = reinterpret_cast<NMHDR*>(lp); // note: not const!
		if (pHdr->code >= HDN_GETDISPINFO && pHdr->code <= HDN_BEGINDRAG) { // all HDN messages
			pHdr->idFrom = GetDlgCtrlID(hLv); // dangerous games: replace Header ID with ListView's, easier for DLGPROC
			SendMessageW(GetAncestor(hLv, GA_PARENT), WM_NOTIFY, wp, lp); // simply forward to parent
		}
	} else if (uMsg == WM_NCDESTROY) {
		RemoveWindowSubclass(hLv, lv_subclass_proc, idSubclass);
	}
	return DefSubclassProc(hLv, uMsg, wp, lp);
}

const ListView& ListView::_raw_activate_mods(HMENU hMenu) const {
	static UINT idSubclass = 1;
	DWORD_PTR param = reinterpret_cast<DWORD_PTR>(hMenu);
	if (BOOL ok = SetWindowSubclass(hwnd(), lv_subclass_proc, idSubclass++, param); !ok) [[unlikely]] {
		throw WinErr{ERROR_UNIDENTIFIED_ERROR, str::fmt(L"SetWindowSubclass failed for ListView ID=%u.", ctrl_id())};
	}
	return *this;
}

const ListView& ListView::_raw_set_image_list(const ImageList &il, DWORD lvsil) const {
	if (!(GetWindowLongPtrW(hwnd(), GWL_STYLE) & LVS_SHAREIMAGELISTS)) [[unlikely]] {
		throw WinErr{ERROR_BAD_ARGUMENTS, L"ListView doesn't have LVS_SHAREIMAGELISTS."};
	}

	ListView_SetImageList(hwnd(), il.himglist(), lvsil);
	return *this;
}

ListView::Item ListView::_raw_item_find(StrView text, int col, bool caseInsensitive) const {
	const int numItems = item_count();
	const int numCols = col_count();

	for (int i = 0; i < numItems; ++i) {
		const Item itemI{*this, i};
		if (col >= 0) { // check only under a specific column
			if (caseInsensitive) {
				if (text.eq_i(itemI.text(col)))
					return itemI;
			} else {
				if (text == itemI.text(col))
					return itemI;
			}
		} else { // check under all cols
			for (int c = 0; c < numCols; ++c) {
				if (caseInsensitive) {
					if (text.eq_i(itemI.text(c)))
						return itemI;
				} else {
					if (text == itemI.text(c))
						return itemI;
				}
			}
		}
	}
	return Item{*this, -1};
}

const ListView& ListView::_set_lvs_ex(DWORD exStyle) const noexcept {
	ListView_SetExtendedListViewStyleEx(hwnd(), exStyle, exStyle);
	return *this;
}
