#pragma once
#include "NativeControl.h"

namespace lib {

// Check box and Radio button native controls.
class CheckRadio final : public NativeControl {
public:
	virtual ~CheckRadio() { }

	constexpr CheckRadio() = default;
	constexpr CheckRadio(const CheckRadio&) = default;
	constexpr CheckRadio(CheckRadio&&) = default;
	constexpr CheckRadio& operator=(const CheckRadio&) = default;
	constexpr CheckRadio& operator=(CheckRadio&&) = default;

	constexpr explicit CheckRadio(HWND hCtrl) : NativeControl{hCtrl} { }
	CheckRadio(HWND hParent, WORD ctrlId) : NativeControl{hParent, ctrlId} { }
	CheckRadio(Window* parent, WORD ctrlId) : NativeControl{parent, ctrlId} { }

	const CheckRadio& check(bool checked = true) const;
	const CheckRadio& checkAndTrigger(bool checked = true) const;
	[[nodiscard]] bool isChecked() const;

private:
	Window::_hWndPtr;
};

}
