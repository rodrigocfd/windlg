#include "ComboBox.h"
using namespace lib;

const ComboBox& ComboBox::add(std::initializer_list<std::wstring_view> items) const
{
	for (auto&& item : items)
		SendMessage(hWnd(), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(item.data()));
	return *this;
}

const ComboBox& ComboBox::clear() const
{
	SendMessageW(hWnd(), CB_RESETCONTENT, 0, 0);
	return *this;
}

UINT ComboBox::count() const
{
	return static_cast<UINT>(SendMessageW(hWnd(), CB_GETCOUNT, 0, 0));
}

const ComboBox& ComboBox::remove(UINT index) const
{
	SendMessageW(hWnd(), CB_DELETESTRING, index, 0);
	return *this;
}

const ComboBox& ComboBox::select(std::optional<UINT> index) const
{
	SendMessageW(hWnd(), CB_SETCURSEL, index.value_or(-1), 0);
	return *this;
}

std::optional<UINT> ComboBox::selectedIndex() const
{
	int i = static_cast<int>(SendMessageW(hWnd(), CB_GETCURSEL, 0, 0));
	return i == CB_ERR ? std::nullopt : std::optional{i};
}

std::optional<std::wstring> ComboBox::selectedText() const
{
	std::optional maybeSelIdx = selectedIndex();
	if (maybeSelIdx.has_value()) {
		UINT selIdx = maybeSelIdx.value();
		size_t len = SendMessageW(hWnd(), CB_GETLBTEXTLEN, selIdx, 0);
		if (!len) return {};

		std::wstring buf(len, L'\0');
		SendMessageW(hWnd(), CB_GETLBTEXT, selIdx, reinterpret_cast<LPARAM>(buf.data()));
		buf.resize(len);
		return {buf};
	} else {
		return std::nullopt;
	}
}
