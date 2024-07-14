#include "CheckRadio.h"
using namespace lib;

const CheckRadio& CheckRadio::check(bool checked) const
{
	SendMessageW(hWnd(), BM_SETCHECK, checked ? BST_CHECKED : BST_UNCHECKED, 0);
	return *this;
}

const CheckRadio& CheckRadio::checkAndTrigger(bool checked) const
{
	check(checked);
	SendMessageW(hWnd(), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hWnd()), BN_CLICKED), reinterpret_cast<LPARAM>(hWnd()));
	return *this;
}

bool CheckRadio::isChecked() const
{
	return SendMessageW(hWnd(), BM_GETCHECK, 0, 0) == BST_CHECKED;
}
