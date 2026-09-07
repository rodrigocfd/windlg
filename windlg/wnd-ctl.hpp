#pragma once
#include "wnd.hpp"
#include <CommCtrl.h>

namespace wd {

	// Calls EnableWindow on multiple child controls.
	void enable_many(const BaseContainer *pParent, bool doEnable, std::initializer_list<WORD> ctrlIds) noexcept;
	// Calls EnableWindow on multiple child controls.
	void enable_many(const BaseContainer *pParent, bool doEnable, std::span<const WORD> ctrlIds) noexcept;

	/// Base to all native child controls.
	class BaseNativeCtrl {
	public:
		virtual ~BaseNativeCtrl() = default;
		constexpr BaseNativeCtrl(const BaseContainer *pParent, WORD ctrlId) noexcept
			: _pParent{pParent}, _ctrlId{ctrlId} { }

		const BaseNativeCtrl& enable(bool doEnable) const noexcept;
		const BaseNativeCtrl& focus() const noexcept;
		[[nodiscard]] HWND hwnd() const noexcept                             { return GetDlgItem(_pParent->hwnd(), _ctrlId); }
		[[nodiscard]] constexpr const BaseContainer* parent() const noexcept { return _pParent; }
		[[nodiscard]] constexpr WORD ctrl_id() const noexcept                { return _ctrlId; }
	private:
		const BaseContainer *_pParent;
		WORD _ctrlId;
	};

	/// Button control.
	class Button : public BaseNativeCtrl {
	public:
		virtual ~Button() = default;
		constexpr Button(const BaseContainer *pParent, WORD ctrlId) noexcept : BaseNativeCtrl{pParent, ctrlId} { }

		[[nodiscard]] std::wstring text() const;
		const Button& set_text(StrView text) const noexcept;
	};

	/// Check box control.
	class CheckBox : public BaseNativeCtrl {
	public:
		virtual ~CheckBox() = default;
		constexpr CheckBox(const BaseContainer *pParent, WORD ctrlId) noexcept : BaseNativeCtrl{pParent, ctrlId} { }

		[[nodiscard]] bool is_checked() const noexcept;
		const CheckBox& set_check(bool doCheck) const noexcept;
		const CheckBox& set_check_and_trigger(bool doCheck) const noexcept;
		[[nodiscard]] std::wstring text() const;
		const CheckBox& set_text(StrView text) const noexcept;
		const CheckBox& set_text_and_resize(StrView text) const;
	};

	/// Combo box control.
	class ComboBox final : public BaseNativeCtrl {
	public:
		/// A single item of the ComboBox.
		class Item final {
		public:
			constexpr Item(const ComboBox &cmb, int index) noexcept
				: _cmb{cmb}, _index{index} { }

			[[nodiscard]] constexpr int index() const noexcept { return _index; }
			[[nodiscard]] ComboBox combo_box() const noexcept  { return ComboBox{_cmb.parent(), _cmb.ctrl_id()}; }
			const Item& del() const noexcept;
			const Item& select() const noexcept;
			const Item& set_text(StrView text) const noexcept;
			[[nodiscard]] std::wstring text() const;

		private:
			BaseNativeCtrl _cmb;
			int _index;
		};

		virtual ~ComboBox() = default;
		constexpr ComboBox(const BaseContainer *pParent, WORD ctrlId) noexcept : BaseNativeCtrl{pParent, ctrlId} { }

		[[nodiscard]] Item item(int index) const noexcept { return Item{*this, index < 0 ? (item_count() + index) : index}; } // Negative indexes are meaningful: -1 is last.
		Item item_add(StrView text) const noexcept;
		const ComboBox& item_add(std::initializer_list<StrView> texts) const noexcept;
		const ComboBox& item_add(std::span<const wchar_t* const> texts) const noexcept;
		const ComboBox& item_add(std::span<const StrView> texts) const noexcept;
		[[nodiscard]] int item_count() const noexcept;
		const ComboBox& item_del_all() const noexcept;
		[[nodiscard]] Item item_selected() const noexcept;
		[[nodiscard]] std::wstring text() const;
	};

	/// Date and time picker control.
	class DateTimePicker final : public BaseNativeCtrl {
	public:
		virtual ~DateTimePicker() = default;
		constexpr DateTimePicker(const BaseContainer *pParent, WORD ctrlId) noexcept : BaseNativeCtrl{pParent, ctrlId} { }

		[[nodiscard]] SYSTEMTIME time() const noexcept;
		const DateTimePicker& set_time(const SYSTEMTIME &st) const noexcept;
		const DateTimePicker& set_time(const FILETIME &ft) const noexcept;
	};

	/// Edit control.
	class Edit final : public Button {
	public:
		virtual ~Edit() = default;
		constexpr Edit(const BaseContainer *pParent, WORD ctrlId) noexcept : Button{pParent, ctrlId} { }
	};

	/// Progress bar control.
	class ProgressBar final : public BaseNativeCtrl {
	public:
		enum class State : WORD { normal=PBST_NORMAL, error=PBST_ERROR, pause=PBST_PAUSED };

		virtual ~ProgressBar() = default;
		constexpr ProgressBar(const BaseContainer *pParent, WORD ctrlId) noexcept : BaseNativeCtrl{pParent, ctrlId} { }

		[[nodiscard]] int pos() const noexcept;
		[[nodiscard]] PBRANGE range() const noexcept;
		ProgressBar& set_marquee(bool isMarquee) noexcept;
		ProgressBar& set_pos(int newPos) noexcept;
		ProgressBar& set_pos(size_t newPos) noexcept                           { return set_pos(static_cast<int>(newPos)); }
		const ProgressBar& set_range(int minVal, int maxVal) const noexcept;
		const ProgressBar& set_range(int minVal, size_t maxVal) const noexcept { return set_range(minVal, static_cast<int>(maxVal)); }
		const ProgressBar& set_state(State state) const noexcept;
	private:
		bool _isMarquee = false;
	};

	/// Radio button control.
	class RadioButton final : public CheckBox {
	public:
		virtual ~RadioButton() = default;
		constexpr RadioButton(const BaseContainer *pParent, WORD ctrlId) noexcept : CheckBox{pParent, ctrlId} { }
	};

	/// Static control.
	class Static final : public BaseNativeCtrl {
	public:
		virtual ~Static() = default;
		constexpr Static(const BaseContainer *pParent, WORD ctrlId) noexcept : BaseNativeCtrl{pParent, ctrlId} { }

		[[nodiscard]] std::wstring text() const;
		const Static& set_text(StrView text) const noexcept;
		const Static& set_text_and_resize(StrView text) const;
	};

	/// Status bar control.
	class StatusBar final : public BaseNativeCtrl {
	public:
		/// A single part of the StatusBar.
		class Part final {
		public:
			constexpr Part(const StatusBar &sb, int index) noexcept
				: _sb{sb}, _index{index} { }

			[[nodiscard]] constexpr int index() const noexcept          { return _index; }
			const Part& set_text(StrView text) const noexcept;
			[[nodiscard]] constexpr const StatusBar& status_bar() const { return _sb; }
			[[nodiscard]] std::wstring text() const;

		private:
			const StatusBar &_sb;
			int _index;
		};

		virtual ~StatusBar() = default;
		explicit StatusBar(const BaseContainer *pParent, WORD ctrlId = 0) noexcept;

		StatusBar(const StatusBar&) = delete;
		StatusBar& operator=(const StatusBar&) = delete; // non-copyable
		StatusBar(StatusBar&&) = delete;
		StatusBar& operator=(StatusBar&&) = delete; // non-movable

		[[nodiscard]] Part part(int index) const noexcept          { return Part{*this, index < 0 ? (static_cast<int>(part_count()) + index) : index}; } // Negative indexes are meaningful: -1 is last.
		StatusBar& part_add_fixed(int sizePixels)                  { return _raw_add_part(sizePixels, 0); }
		StatusBar& part_add_resizable(int resizeWeight)            { return _raw_add_part(0, resizeWeight); }
		[[nodiscard]] constexpr size_t part_count() const noexcept { return _partsData.size(); }
		StatusBar& resize_to_parent(WPARAM wp, LPARAM lp) noexcept;

	private:
		StatusBar& _raw_add_part(int sizePixels, int resizeWeight);
		[[nodiscard]] constexpr bool _part_is_fixed(size_t index) const noexcept { return _partsData[index].sizePixels > 0; }

		struct PartData final {
			int sizePixels, resizeWeight;
		};
		std::vector<PartData> _partsData{};
		std::vector<int> _rightEdges{}; // to speed up resize operations
	};

}
