#pragma once
#include "Window.h"

namespace lib {

// Base to all native controls.
class NativeControl : public Window {
public:
	virtual ~NativeControl() { }

	constexpr NativeControl() = default;
	constexpr NativeControl(const NativeControl&) = default;
	constexpr NativeControl(NativeControl&&) = default;
	constexpr NativeControl& operator=(const NativeControl&) = default;
	constexpr NativeControl& operator=(NativeControl&&) = default;

	constexpr explicit NativeControl(HWND hCtrl) : Window{hCtrl} { }
	NativeControl(HWND hParent, WORD ctrlId);
	NativeControl(Window* parent, WORD ctrlId) : NativeControl{parent->hWnd(), ctrlId} { }

	// Sends WM_NEXTDLGCTL.
	void focus() const;
};

}
