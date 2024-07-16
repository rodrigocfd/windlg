#pragma once
#include <functional>
#include <optional>
#include <string>
#include <vector>
#include <Windows.h>
#include "Dialog.h"
#include "ImgList.h"
#include "NativeControl.h"

namespace lib {

// ListView native control.
class ListView final : public NativeControl {
public:
	class Column final {
	private:
		HWND _hList = nullptr;
		int _index = -1;

	public:
		Column() = delete;
		constexpr Column(const Column&) = default;
		constexpr Column(Column&&) = default;
		constexpr Column& operator=(const Column&) = default;
		Column& operator=(Column&&) = delete;

		constexpr Column(HWND hList, int index) : _hList{hList}, _index{index} { }
		constexpr Column(ListView* owner, int index) : Column{owner->hWnd(), _index} { }

		[[nodiscard]] constexpr int index() const { return _index; }
		[[nodiscard]] std::vector<std::wstring> itemTexts() const;
		[[nodiscard]] int justification() const;
		[[nodiscard]] std::vector<std::wstring> selectedItemTexts() const;
		const Column& setJustification(WORD hdf) const;
		const Column& setSortArrow(int hdf) const;
		const Column& setText(std::wstring_view text) const;
		const Column& setWidth(UINT width) const;
		const Column& setWidthToFill() const;
		[[nodiscard]] int sortArrow() const;
		[[nodiscard]] std::wstring text() const;
		[[nodiscard]] UINT width() const;
	};

private:
	class ColumnCollection final {
	private:
		friend ListView;
		HWND _hList = nullptr;

		constexpr explicit ColumnCollection(HWND hList) : _hList{hList} { }

	public:
		ColumnCollection() = delete;
		constexpr ColumnCollection(const ColumnCollection&) = default;
		ColumnCollection(ColumnCollection&&) = delete;
		constexpr ColumnCollection& operator=(const ColumnCollection&) = default;
		ColumnCollection& operator=(ColumnCollection&&) = delete;

		[[nodiscard]] constexpr Column operator[](int index) const { return Column{_hList, index}; }

		Column add(std::wstring_view text, UINT width) const;
		const ColumnCollection& add(std::initializer_list<std::pair<std::wstring_view, UINT>> namesAndWidths) const;
		[[nodiscard]] UINT count() const;
	};

public:
	class Item final {
	private:
		HWND _hList = nullptr;
		int _index = -1;

	public:
		Item() = delete;
		constexpr Item(const Item&) = default;
		constexpr Item(Item&&) = default;
		constexpr Item& operator=(const Item&) = default;
		Item& operator=(Item&&) = delete;

		constexpr Item(HWND hList, int index) : _hList{hList}, _index{index} { }
		constexpr Item(ListView* owner, int index) : Item{owner->hWnd(), index} { }

		template<typename T> [[nodiscard]] T data() const { if constexpr (std::is_pointer_v<T>) return reinterpret_cast<T>(_data()); else return static_cast<T>(_data()); }
		const Item& focus() const;
		[[nodiscard]] constexpr int index() const { return _index; }
		[[nodiscard]] bool isVisible() const;
		void remove() const;
		const Item& select(bool doSelect = true) const;
		template<typename T> const Item& setData(T v) const { if constexpr (std::is_pointer_v<T>) return _setData(reinterpret_cast<LPARAM>(v)); else return _setData(static_cast<LPARAM>(v)); }
		const Item& setText(std::wstring_view text, UINT columnIndex = 0) const;
		[[nodiscard]] std::wstring text(UINT columnIndex = 0) const;

	private:
		LPARAM _data() const;
		const Item& _setData(LPARAM data) const;
	};

private:
	class ItemCollection final {
	private:
		friend ListView;
		HWND _hList = nullptr;

		constexpr explicit ItemCollection(HWND hList) : _hList{hList} { }

	public:
		ItemCollection() = delete;
		constexpr ItemCollection(const ItemCollection&) = default;
		ItemCollection(ItemCollection&&) = delete;
		constexpr ItemCollection& operator=(const ItemCollection&) = default;
		ItemCollection& operator=(ItemCollection&&) = delete;

		[[nodiscard]] constexpr Item operator[](int index) const { return Item{_hList, index}; }

		Item add(std::wstring_view text,
			std::initializer_list<std::wstring_view> otherColumnsTexts = {}, int icon = -1) const;
		[[nodiscard]] std::vector<Item> all() const;
		[[nodiscard]] UINT count() const;
		[[nodiscard]] UINT countSelected() const;
		[[nodiscard]] std::optional<Item> find(std::wstring_view text) const;
		[[nodiscard]] std::optional<Item> focused() const;
		[[nodiscard]] std::optional<Item> hitTest(int x, int y) const;
		void removeAll() const;
		void removeSelected() const;
		void selectAll(bool doSelect = true) const;
		[[nodiscard]] std::vector<Item> selected() const;
		void sort(std::function<int(Item, Item)> callback) const;
	};

public:
	ColumnCollection columns{hWnd()};
	ItemCollection items{hWnd()};

	virtual ~ListView() { }

	constexpr ListView() = default;
	constexpr ListView(const ListView&) = default;
	ListView(ListView&&) = delete;
	constexpr ListView& operator=(const ListView&) = default;
	ListView& operator=(ListView&&) = delete;

	constexpr explicit ListView(HWND hList) : NativeControl{hList} { }
	ListView(HWND hParent, WORD listId) : NativeControl{hParent, listId} { }
	ListView(Window* parent, WORD listId) : NativeControl{parent, listId} { }

	const ListView& setFullRowSelect(bool doSet = true) const;
	const ListView& setImageList(const ImgList& imgList) const;
	const ListView& setSubclassBehavior() const;

	static void ProcessMessages(Dialog* parent, WORD idList, UINT uMsg, WPARAM wp, LPARAM lp, WORD contextMenuId = 0);

private:
	void _showContextMenu(WORD contextMenuId, bool followCursor, bool hasCtrl, bool hasShift) const;
	static LRESULT CALLBACK _SubclassProc(HWND hList, UINT uMsg,
		WPARAM wp, LPARAM lp, UINT_PTR idSubclass, DWORD_PTR refData);
	Window::_hWndPtr;
};

}
