#pragma once
#include <optional>
#include <string>
#include "NativeControl.h"

namespace lib {

// Combo box native control.
class ComboBox final : public NativeControl {
public:
	virtual ~ComboBox() { }

	constexpr ComboBox() = default;
	constexpr ComboBox(const ComboBox&) = default;
	ComboBox(ComboBox&&) = delete;
	constexpr ComboBox& operator=(const ComboBox&) = default;
	ComboBox& operator=(ComboBox&&) = delete;

	constexpr explicit ComboBox(HWND hCtrl) : NativeControl{hCtrl} { }
	ComboBox(HWND hParent, WORD ctrlId) : NativeControl{hParent, ctrlId} { }
	ComboBox(Window* parent, WORD ctrlId) : NativeControl{parent, ctrlId} { }

	const ComboBox& add(std::initializer_list<std::wstring_view> items) const;
	const ComboBox& clear() const;
	[[nodiscard]] UINT count() const;
	const ComboBox& remove(UINT index) const;
	const ComboBox& select(std::optional<UINT> index) const;
	[[nodiscard]] std::optional<UINT> selectedIndex() const;
	[[nodiscard]] std::optional<std::wstring> selectedText() const;

private:
	Window::_hWndPtr;
};

}
