#pragma once
#include <Windows.h>
#include "Dialog.h"

namespace lib {

// Base to a custom child control.
class CustomControl : public Window {
public:
	virtual ~CustomControl() { }

	constexpr CustomControl() = default;
	CustomControl(const CustomControl&) = delete;
	CustomControl(CustomControl&&) = delete;
	CustomControl& operator=(const CustomControl&) = delete;
	CustomControl& operator=(CustomControl&&) = delete;

	// Creates the custom child control and returns immediately.
	void create(Dialog* parent, int x, int y, UINT cx, UINT cy,
		DWORD bgColor = COLOR_WINDOW,
		DWORD style = WS_CHILD | WS_TABSTOP | WS_GROUP | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		DWORD exStyle = WS_EX_LEFT | WS_EX_CLIENTEDGE,
		WORD ctrlId = 0) const;

protected:
	virtual LRESULT wndProc(UINT uMsg, WPARAM wp, LPARAM lp) = 0; // to be overriden in user class

private:
	static LRESULT CALLBACK _WndProc(HWND hWnd, UINT uMsg, WPARAM wp, LPARAM lp);
	void _paintControlBorders(WPARAM wp, LPARAM lp);
};

}
