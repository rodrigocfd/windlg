#pragma once
#include "wnd-ctl.hpp"
#include "wnd-aux.hpp"

namespace wd {

	/// List view control.
	class ListView final : public BaseNativeCtrl {
	public:
		enum class Align : WORD { left=HDF_LEFT, right=HDF_RIGHT, center=HDF_CENTER };
		enum class Arrow : WORD { none=0, asc=HDF_SORTUP, desc=HDF_SORTDOWN };

		/// A single column of the ListView.
		class Col final {
		public:
			constexpr Col(const ListView &lv, int index) noexcept
				: _lv{lv}, _index{index} { }

			[[nodiscard]] constexpr int index() const noexcept  { return _index; }
			[[nodiscard]] ListView list_view() const noexcept   { return ListView{_lv.parent(), _lv.ctrl_id()}; }
			[[nodiscard]] std::vector<std::wstring> selected_texts() const;
			const Col& set_align(Align align) const noexcept;
			const Col& set_arrow(Arrow arrow) const noexcept;
			const Col& set_width(UINT width) const noexcept;
			const Col& set_width_to_fill() const noexcept;
			[[nodiscard]] std::vector<std::wstring> texts() const;
			[[nodiscard]] constexpr bool valid() const noexcept { return _index >= 0; }
			[[nodiscard]] UINT width() const noexcept;

		private:
			BaseNativeCtrl _lv;
			int _index;
		};

		/// A single item of the ListView.
		class Item final {
		public:
			constexpr Item(const ListView &lv, int index) noexcept
				: _lv{lv}, _index{index} { }

			const Item& del() const noexcept;
			const Item& focus() const noexcept;
			[[nodiscard]] int icon() const noexcept;
			[[nodiscard]] constexpr int index() const noexcept  { return _index; }
			[[nodiscard]] bool is_focused() const noexcept;
			[[nodiscard]] bool is_selected() const noexcept;
			[[nodiscard]] bool is_visible() const noexcept;
			[[nodiscard]] ListView list_view() const noexcept   { return ListView{_lv.parent(), _lv.ctrl_id()}; }
			const Item& select(bool doSelect) const noexcept;
			const Item& set_icon(int iconIndex) const noexcept;
			const Item& set_text(int col, StrView text) const noexcept;
			const Item& set_texts(std::initializer_list<StrView> texts) const noexcept;
			[[nodiscard]] std::wstring text(int col) const;
			[[nodiscard]] constexpr bool valid() const noexcept { return _index >= 0; }

			template<typename T>
			[[nodiscard]] T data() const noexcept {
				if constexpr (std::is_pointer_v<T>) {
					return reinterpret_cast<T>(_raw_data());
				} else {
					return static_cast<T>(_raw_data());
				}
			}
			template<typename T>
			const Item& set_data(T value) const noexcept {
				if constexpr (std::is_pointer_v<T>) {
					return _raw_set_data(reinterpret_cast<LPARAM>(value));
				} else {
					return _raw_set_data(static_cast<LPARAM>(value));
				}
			}

		private:
			[[nodiscard]] LPARAM _raw_data() const noexcept;
			const Item& _raw_set_data(LPARAM data) const noexcept;

			BaseNativeCtrl _lv;
			int _index;
		};

		virtual ~ListView() = default;
		constexpr ListView(const BaseContainer *pParent, WORD ctrlId) noexcept
			: BaseNativeCtrl{pParent, ctrlId} { }

		const ListView& activate_mods() const                            { return _raw_activate_mods(nullptr); }       // Subclasses the ListView to implement multiple behaviors.
		const ListView& activate_mods(const Menu &popup) const           { return _raw_activate_mods(popup.hmenu()); } // Subclasses the ListView to implement multiple behaviors.
		const ListView& set_full_row_sel() const noexcept                { return _set_lvs_ex(LVS_EX_FULLROWSELECT); }
		const ListView& set_grid_lines() const noexcept                  { return _set_lvs_ex(LVS_EX_GRIDLINES); }
		const ListView& set_image_list16(const ImageList &il) const      { return _raw_set_image_list(il, LVSIL_SMALL); }  // Calls ListView_SetImageList().
		const ListView& set_image_list32(const ImageList &il) const      { return _raw_set_image_list(il, LVSIL_NORMAL); } // Calls ListView_SetImageList().

		[[nodiscard]] Col col(int index) const noexcept                  { return Col{*this, index < 0 ? (col_count() + index) : index}; } // Negative indexes are meaningful: -1 is last.
		const ListView& col_add(StrView text, UINT width) const noexcept;
		const ListView& col_add(StrView text, UINT width, Align align) const noexcept;
		[[nodiscard]] int col_count() const noexcept;

		[[nodiscard]] Item item(int index) const noexcept                { return Item{*this, index < 0 ? (item_count() + index) : index}; } // Negative indexes are meaningful: -1 is last.
		Item item_add(std::initializer_list<StrView> texts, int iconIndex = -1) const;
		[[nodiscard]] int item_count() const noexcept;
		const ListView& item_del_all() const noexcept;
		const ListView& item_del_selected() const noexcept;
		[[nodiscard]] Item item_find(StrView text, int col = -1) const   { return _raw_item_find(text, col, false); }
		[[nodiscard]] Item item_find_i(StrView text, int col = -1) const { return _raw_item_find(text, col, true); }
		[[nodiscard]] Item item_focused() const noexcept;
		const ListView& item_select_all(bool doSelect) const noexcept;
		[[nodiscard]] std::vector<Item> item_selected() const;
		[[nodiscard]] int item_selected_count() const noexcept;

		enum class Sort { asc, desc, asc_i, desc_i };
		/// Specifies the ListView column sorting.
		struct SortCriterion final {
			int index;
			Sort sort;
		};
		const ListView& sort(SortCriterion criterion) const              { return sort({criterion}); }
		const ListView& sort(std::initializer_list<SortCriterion> criteria) const;
		const ListView& sort(std::function<int(Item a, Item b)> cb) const;

	private:
		const ListView& _raw_activate_mods(HMENU hMenu) const;
		const ListView& _raw_set_image_list(const ImageList &il, DWORD lvsil) const;
		[[nodiscard]] Item _raw_item_find(StrView text, int col, bool caseInsensitive) const;
		const ListView& _set_lvs_ex(DWORD exStyle) const noexcept;
	};

}
