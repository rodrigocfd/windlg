#include "NativeControl.h"
using namespace lib;

NativeControl::NativeControl(HWND hParent, WORD ctrlId)
	: Window{GetDlgItem(hParent, ctrlId)}
{
}

void NativeControl::focus() const
{
	SendMessageW(GetParent(hWnd()), WM_NEXTDLGCTL, reinterpret_cast<WPARAM>(hWnd()), MAKELPARAM(TRUE, 0));
}
