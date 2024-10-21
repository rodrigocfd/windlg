#include <system_error>
#include <Windows.h>
#include <CommCtrl.h>
#include "ListView.h"
#include "Menu.h"
#include "str.h"
using namespace lib;

std::vector<std::wstring> ListView::Column::itemTexts() const
{
	UINT num = ListView{_hList}.items.count();
	std::vector<std::wstring> texts;
	texts.reserve(num);

	for (UINT i = 0; i < num; ++i)
		texts.emplace_back(ListView{_hList}.items[i].text(_index));

	return texts;
}

int ListView::Column::justification() const
{
	HWND hHeader = ListView_GetHeader(_hList);

	HDITEMW hdi = {.mask = HDI_FORMAT};
	Header_GetItem(hHeader, _index, &hdi);

	return hdi.fmt & (HDF_LEFT | HDF_CENTER | HDF_RIGHT);
}

std::vector<std::wstring> ListView::Column::selectedItemTexts() const
{
	std::vector<std::wstring> texts;
	texts.reserve(ListView{_hList}.items.countSelected());

	int idx = -1;
	for (;;) {
		idx = ListView_GetNextItem(_hList, idx, LVNI_SELECTED);
		if (idx == -1) return texts;
		texts.emplace_back(ListView{_hList}.items[idx].text(_index));
	}
}

const ListView::Column& ListView::Column::setJustification(WORD hdf) const
{
	HWND hHeader = ListView_GetHeader(_hList);

	HDITEMW hdi = {.mask = HDI_FORMAT};
	Header_GetItem(hHeader, _index, &hdi);

	hdi.fmt &= ~(HDF_CENTER | HDF_LEFT | HDF_RIGHT); // clear all three
	hdi.fmt |= hdf;
	Header_SetItem(hHeader, _index, &hdi);

	return *this;
}

const ListView::Column& ListView::Column::setSortArrow(int hdf) const
{
	HWND hHeader = ListView_GetHeader(_hList);
	UINT numCols = Header_GetItemCount(hHeader);

	for (UINT i = 0; i < numCols; ++i) {
		HDITEMW hdi = {.mask = HDI_FORMAT};
		Header_GetItem(hHeader, i, &hdi);

		hdi.fmt &= ~(HDF_SORTDOWN | HDF_SORTUP); // clear all two
		if (i == _index) // only the targeted column will have the flag set
			hdi.fmt |= hdf;

		Header_SetItem(hHeader, i, &hdi);
	}

	return *this;
}

const ListView::Column& ListView::Column::setText(std::wstring_view text) const
{
	LVCOLUMNW lvc = {
		.mask = LVCF_TEXT,
		.pszText = const_cast<LPWSTR>(text.data()),
	};

	ListView_SetColumn(_hList, _index, &lvc);
	return *this;
}

const ListView::Column& ListView::Column::setWidth(UINT width) const
{
	ListView_SetColumnWidth(_hList, _index, width);
	return *this;
}

const ListView::Column& ListView::Column::setWidthToFill() const
{
	UINT numCols = ListView{_hList}.columns.count();
	if (numCols == 0) return *this;

	UINT cxUsed = 0;
	for (UINT i = 0; i < numCols; ++i) {
		if (i != _index)
			cxUsed += ListView{_hList}.columns[i].width(); // retrieve cx of each column, but us
	}

	RECT rc{};
	GetClientRect(_hList, &rc); // list view client area
	return setWidth(rc.right - cxUsed);
}

int ListView::Column::sortArrow() const
{
	HWND hHeader = ListView_GetHeader(_hList);

	HDITEMW hdi = {.mask = HDI_FORMAT};
	Header_GetItem(hHeader, _index, &hdi);

	return hdi.fmt & (HDF_SORTDOWN | HDF_SORTUP);
}

std::wstring ListView::Column::text() const
{
	std::wstring buf(128, L'\0'); // arbitrary

	LVCOLUMNW lvc = {
		.mask = LVCF_TEXT,
		.pszText = buf.data(),
		.cchTextMax = 128,
	};

	ListView_GetColumn(_hList, _index, &lvc);
	str::trimNulls(buf);
	return buf;
}

UINT ListView::Column::width() const
{
	return ListView_GetColumnWidth(_hList, _index);
}


ListView::Column ListView::ColumnCollection::add(std::wstring_view text, UINT width) const
{
	LVCOLUMNW lvc = {
		.mask = LVCF_TEXT | LVCF_WIDTH,
		.cx = static_cast<int>(width),
		.pszText = const_cast<LPWSTR>(text.data()),
	};
	int index = ListView_InsertColumn(_hList, 0xffff, &lvc); // insert as the last column
	return Column{_hList, index}; // return newly added column
}

const ListView::ColumnCollection& ListView::ColumnCollection::add(
	std::initializer_list<std::pair<std::wstring_view, UINT>> namesAndWidths) const
{
	LVCOLUMNW lvc = {.mask = LVCF_TEXT | LVCF_WIDTH};

	for (const auto& nameWidth : namesAndWidths) {
		lvc.cx = nameWidth.second;
		lvc.pszText = const_cast<LPWSTR>(nameWidth.first.data());
		ListView_InsertColumn(_hList, 0xffff, &lvc); // insert as the last column
	}
	return *this;
}

UINT ListView::ColumnCollection::count() const
{
	HWND hHeader = ListView_GetHeader(_hList);
	return Header_GetItemCount(hHeader);
}


const ListView::Item& ListView::Item::focus() const
{
	ListView_SetItemState(_hList, _index, LVIS_FOCUSED, LVIS_FOCUSED);
	return *this;
}

const ListView::Item& ListView::Item::select(bool doSelect) const
{
	ListView_SetItemState(_hList, _index, doSelect ? LVIS_SELECTED : 0, LVIS_SELECTED);
	return *this;
}

bool ListView::Item::isSelected() const
{
	return ListView_GetItemState(_hList, _index, LVIS_SELECTED) & LVIS_SELECTED;
}

bool ListView::Item::isVisible() const
{
	return ListView_IsItemVisible(_hList, _index);
}

void ListView::Item::remove() const
{
	ListView_DeleteItem(_hList, _index);
}

const ListView::Item& ListView::Item::setText(std::wstring_view text, UINT columnIndex) const
{
	ListView_SetItemText(_hList, _index, columnIndex, const_cast<LPWSTR>(text.data()));
	return *this;
}

std::wstring ListView::Item::text(UINT columnIndex) const
{
	UINT curBufSz = str::SSO_LEN;
	std::wstring buf(curBufSz, L'\0');

	for (;;) {
		LVITEMW lvi = {
			.mask = LVIF_TEXT,
			.iSubItem = static_cast<int>(columnIndex),
			.pszText = buf.data(),
			.cchTextMax = static_cast<int>(curBufSz),
		};

		UINT recvChars = static_cast<UINT>(
			SendMessageW(_hList, LVM_GETITEMTEXTW, _index, reinterpret_cast<LPARAM>(&lvi)));
		recvChars += 1; // plus terminating null count

		if (recvChars < curBufSz) { // to break, must have at least 1 char gap
			str::trimNulls(buf);
			return buf;
		}

		curBufSz *= 2; // double the buffer size to try again
		buf.resize(curBufSz, L'\0');
	}
}

LPARAM ListView::Item::_data() const
{
	LVITEMW lvi = {
		.mask = LVIF_PARAM,
		.iItem = _index,
	};
	ListView_GetItem(_hList, &lvi);
	return lvi.lParam;
}

const ListView::Item& ListView::Item::_setData(LPARAM data) const
{
	LVITEMW lvi = {
		.mask = LVIF_PARAM,
		.iItem = _index,
		.lParam = data,
	};
	ListView_SetItem(_hList, &lvi);
	return *this;
}


ListView::Item ListView::ItemCollection::add(std::wstring_view text,
	std::initializer_list<std::wstring_view> otherColumnsTexts, int icon) const
{
	LVITEMW lvi = {
		.mask = LVIF_TEXT | static_cast<UINT>(icon != -1 ? LVIF_IMAGE : 0),
		.iItem = 0x0fff'ffff, // insert as the last item
		.pszText = const_cast<LPWSTR>(text.data()),
		.iImage = icon,
	};
	int index = ListView_InsertItem(_hList, &lvi);
	Item newItem{_hList, index};

	for (auto&& colText = otherColumnsTexts.begin(); colText != otherColumnsTexts.end(); ++colText) {
		size_t idx = std::distance(otherColumnsTexts.begin(), colText);
		newItem.setText(*colText, static_cast<UINT>(idx) + 1);
	}

	return newItem; // return the newly added item
}

std::vector<ListView::Item> ListView::ItemCollection::all() const
{
	UINT num = count();
	std::vector<Item> items;
	items.reserve(num);

	for (UINT i = 0; i < num; ++i)
		items.emplace_back(_hList, i);

	return items;
}

UINT ListView::ItemCollection::count() const
{
	return ListView_GetItemCount(_hList);
}

UINT ListView::ItemCollection::countSelected() const
{
	return ListView_GetSelectedCount(_hList);
}

std::optional<ListView::Item> ListView::ItemCollection::find(std::wstring_view text) const
{
	LVFINDINFOW fi = {
		.flags = LVFI_STRING,
		.psz = text.data(),
	};
	int idx = ListView_FindItem(_hList, -1, &fi);
	return idx == -1 ? std::nullopt : std::optional{Item{_hList, idx}};
}

std::optional<ListView::Item> ListView::ItemCollection::focused() const
{
	int idx = ListView_GetNextItem(_hList, -1, LVNI_FOCUSED);
	return (idx == -1) ? std::nullopt : std::optional{Item{_hList, idx}};
}

std::optional<ListView::Item> ListView::ItemCollection::hitTest(int x, int y) const
{
	LVHITTESTINFO hti = {
		.pt = {.x = x, .y = y},
	};
	int idx = ListView_HitTest(_hList, &hti);
	return (idx == -1) ? std::nullopt : std::optional{Item{_hList, idx}};
}

void ListView::ItemCollection::removeAll() const
{
	ListView_DeleteAllItems(_hList);
}

void ListView::ItemCollection::removeSelected() const
{
	for (;;) {
		int idx = ListView_GetNextItem(_hList, -1, LVNI_SELECTED);
		if (idx == -1) break;
		ListView_DeleteItem(_hList, idx);
	}
}

void ListView::ItemCollection::selectAll(bool doSelect) const
{
	ListView_SetItemState(_hList, -1, doSelect ? LVIS_SELECTED : 0, LVIS_SELECTED);
}

std::vector<ListView::Item> ListView::ItemCollection::selected() const
{
	std::vector<Item> items;
	items.reserve(this->countSelected());

	int idx = -1;
	for (;;) {
		idx = ListView_GetNextItem(_hList, idx, LVNI_SELECTED);
		if (idx == -1) return items;
		items.emplace_back(_hList, idx);
	}
}

void ListView::ItemCollection::sort(std::function<int(Item, Item)> callback) const
{
	struct Info final {
		ListView lv;
		std::function<int(Item, Item)> callback;
	};
	Info nfo = {.lv = ListView{_hList}, .callback = std::move(callback)};

	ListView_SortItemsEx(_hList, [](LPARAM idxA, LPARAM idxB, LPARAM lp) -> int { // receives indexes
		Info* pNfo = reinterpret_cast<Info*>(lp);
		return pNfo->callback(pNfo->lv.items[static_cast<int>(idxA)],
			pNfo->lv.items[static_cast<int>(idxB)]);
	}, &nfo);
}


const ListView& ListView::setFullRowSelect(bool doSet) const
{
	ListView_SetExtendedListViewStyleEx(hWnd(), LVS_EX_FULLROWSELECT, doSet ? LVS_EX_FULLROWSELECT : 0);
	return *this;
}

const ListView& ListView::setGridLines(bool doSet) const
{
	ListView_SetExtendedListViewStyleEx(hWnd(), LVS_EX_GRIDLINES, doSet ? LVS_EX_GRIDLINES : 0);
	return *this;
}

const ListView& ListView::setImageList(const ImgList& imgList) const
{
	SIZE res = imgList.resolution();
	int lvsil = 0;

	switch (res.cx) {
		case 16: lvsil = LVSIL_SMALL; break;
		case 32: lvsil = LVSIL_NORMAL; break;
		default: throw std::logic_error("Invalid image list resolution");
	}

	ListView_SetImageList(hWnd(), imgList.hImg(), lvsil);
	return *this;
}

void ListView::ProcessMessages(Dialog *parent, WORD idList, UINT uMsg, WPARAM wp, LPARAM lp, WORD contextMenuId)
{
	if (uMsg == WM_INITDIALOG) {
		if (ListView lv{parent, idList}; lv.hWnd()) [[likely]] {
#ifdef _DEBUG
			DWORD exStyle = static_cast<DWORD>(GetWindowLongPtrW(lv.hWnd(), GWL_STYLE));
			if ( !(exStyle & LVS_SHAREIMAGELISTS) )
				throw std::runtime_error(lib::str::toAnsi(lib::str::fmt(L"ListView %d doesn't have LVS_SHAREIMAGELISTS.", idList)));
#endif

			static UINT idSubclass = 1;
			if (!SetWindowSubclass(lv.hWnd(), _SubclassProc, idSubclass++, 0)) [[unlikely]] {
				throw std::system_error(GetLastError(), std::system_category(), "SetWindowSubclass failed");
			}
		}
	} else if (uMsg == WM_NOTIFY) {
		if (ListView lv{parent, idList}; lv.hWnd()) [[likely]] {
			if (NMHDR* hdr = reinterpret_cast<NMHDR*>(lp); hdr->hwndFrom == lv.hWnd()) {
				if (hdr->code == LVN_KEYDOWN) {
					NMLVKEYDOWN* lvkd = reinterpret_cast<NMLVKEYDOWN*>(lp);
					bool hasCtrl = GetAsyncKeyState(VK_CONTROL);
					bool hasShift = GetAsyncKeyState(VK_SHIFT);
					if (hasCtrl && lvkd->wVKey == 'A') { // Ctrl+A
						lv.items.selectAll();
					} else if (contextMenuId && lvkd->wVKey == VK_APPS) { // context menu key
						lv._showContextMenu(contextMenuId, false, hasCtrl, hasShift);
					}
				} else if (hdr->code == NM_RCLICK && contextMenuId) {
					NMITEMACTIVATE* ia = reinterpret_cast<NMITEMACTIVATE*>(lp);
					bool hasCtrl = (ia->uKeyFlags & LVKF_CONTROL) != 0;
					bool hasShift = (ia->uKeyFlags & LVKF_SHIFT) != 0;
					lv._showContextMenu(contextMenuId, true, hasCtrl, hasShift);
				}
			}
		}
	}
}

void ListView::_showContextMenu(WORD contextMenuId, bool followCursor, bool hasCtrl, bool hasShift) const
{
	POINT menuPos{};

	if (followCursor) { // usually when fired by a right-click
		GetCursorPos(&menuPos); // relative to screen
		ScreenToClient(hWnd(), &menuPos); // now relative to listview
		std::optional<Item> itemOver = items.hitTest(menuPos.x, menuPos.y);
		if (!itemOver.has_value()) { // no item was right-clicked
			items.selectAll(false);
		} else if (!hasCtrl && !hasShift) {
			itemOver.value().select().focus(); // if note yet
		}
		SetFocus(hWnd()); // because a right-click won't set the focus by itself
	} else { // usually fired by the context meny key
		std::optional<Item> itemFocused = items.focused();
		if (itemFocused.has_value() && itemFocused.value().isVisible()) {
			RECT rc = {.left = LVIR_BOUNDS};
			SendMessageW(hWnd(), LVM_GETITEMRECT, itemFocused.value().index(), reinterpret_cast<LPARAM>(&rc));
			menuPos = {.x = rc.left + 16, .y = rc.top + (rc.bottom - rc.top) / 2};
		} else { // no item is focused and visible
			menuPos = {.x = 6, .y = 10}; // arbitrary coordinates
		}
	}

	HWND hParent = GetParent(hWnd());
	HINSTANCE hInst = reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(hParent, GWLP_HINSTANCE));
	
	Menu menu{hInst, contextMenuId};
	menu.subMenu(0).showAtPoint(menuPos.x, menuPos.y, hParent, hWnd());
	menu.destroy();
}

LRESULT CALLBACK ListView::_SubclassProc(HWND hList, UINT uMsg,
	WPARAM wp, LPARAM lp, UINT_PTR idSubclass, DWORD_PTR refData)
{
	if (uMsg == WM_GETDLGCODE) {
		if (lp && wp == VK_RETURN) { // Enter key
			WORD ctrlId = GetDlgCtrlID(hList);
			NMLVKEYDOWN nmlvkd = {
				.hdr = {
					.hwndFrom = hList,
					.idFrom = ctrlId,
					.code = LVN_KEYDOWN,
				},
				.wVKey = VK_RETURN,
			};
			SendMessageW(GetAncestor(hList, GA_PARENT), WM_NOTIFY, ctrlId,
				reinterpret_cast<LPARAM>(&nmlvkd)); // forward to parent

			LRESULT dlgcSystem = DefSubclassProc(hList, WM_GETDLGCODE, wp, lp);
			return dlgcSystem;
		}
	} else if (uMsg == WM_NOTIFY) {
		NMHDR* hdr = reinterpret_cast<NMHDR*>(lp);
		if (hdr->code >= HDN_GETDISPINFO && hdr->code <= HDN_BEGINDRAG) { // all HDN messages
			hdr->idFrom = GetDlgCtrlID(hList); // replace Header ID with ListView's, easier for DLGPROC
			SendMessageW(GetAncestor(hList, GA_PARENT), WM_NOTIFY, wp, lp); // simply forward to parent
		}
	} else if (uMsg == WM_NCDESTROY) {
		RemoveWindowSubclass(hList, _SubclassProc, idSubclass);
	}
	return DefSubclassProc(hList, uMsg, wp, lp);
}
