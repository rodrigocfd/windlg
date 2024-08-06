#include <system_error>
#include "DialogModal.h"
using namespace lib;

INT_PTR DialogModal::showModal(const Dialog* parent, WORD dlgId) const
{
	if (!parent) [[unlikely]] {
		throw std::invalid_argument("Modal parent is null");
	}

	return DialogBoxParamW(reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(parent->hWnd(), GWLP_HINSTANCE)),
		MAKEINTRESOURCEW(dlgId), parent->hWnd(), Dialog::_DlgProc, reinterpret_cast<LPARAM>(this));
}
