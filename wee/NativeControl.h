#pragma once
#include "Window.h"

namespace lib {

// Base to all native controls.
class NativeControl : public Window {
public:
	virtual ~NativeControl() { }

	constexpr NativeControl() = default;
	constexpr NativeControl(const NativeControl&) = default;
	NativeControl(NativeControl&&) = delete;
	constexpr NativeControl& operator=(const NativeControl&) = default;
	NativeControl& operator=(NativeControl&&) = delete;

	constexpr explicit NativeControl(HWND hCtrl) : Window{hCtrl} { }
	NativeControl(HWND hParent, WORD ctrlId);
	NativeControl(Window* parent, WORD ctrlId) : NativeControl{parent->hWnd(), ctrlId} { }

	// Sends WM_NEXTDLGCTL.
	void focus() const;
};

}
