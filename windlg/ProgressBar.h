#pragma once
#include <Windows.h>
#include <CommCtrl.h>
#include "NativeControl.h"

namespace lib {

// Progress bar native control.
class ProgressBar final : public NativeControl {
public:
	virtual ~ProgressBar() { }

	constexpr ProgressBar() = default;
	constexpr ProgressBar(const ProgressBar&) = default;
	constexpr ProgressBar(ProgressBar&&) = default;
	constexpr ProgressBar& operator=(const ProgressBar&) = default;
	constexpr ProgressBar& operator=(ProgressBar&&) = default;

	constexpr explicit ProgressBar(HWND hCtrl) : NativeControl{hCtrl} { }
	ProgressBar(HWND hParent, WORD ctrlId) : NativeControl{hParent, ctrlId} { }
	ProgressBar(const Window* parent, WORD ctrlId) : NativeControl{parent, ctrlId} { }

	[[nodiscard]] UINT pos() const;
	[[nodiscard]] PBRANGE range() const;
	const ProgressBar& setMarquee(bool marquee = true) const;
	const ProgressBar& setPos(UINT pos) const;
	const ProgressBar& setRange(UINT min, UINT max) const;
	
	// Sets PBST_NORMAL, PBST_ERROR or PBST_PAUSED.
	const ProgressBar& setState(DWORD pbst) const;

	// Returns PBST_NORMAL, PBST_ERROR or PBST_PAUSED.
	[[nodiscard]] DWORD state() const;

private:
	Window::_hWndPtr;
};

}
