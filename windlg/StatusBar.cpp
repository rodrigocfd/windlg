#include <system_error>
#include <Windows.h>
#include <CommCtrl.h>
#include "StatusBar.h"
#include "str.h"
using namespace lib;

const StatusBar::Part& StatusBar::Part::setIcon(HICON hIcon) const
{
	SendMessageW(_hSb, SB_SETICON, _index, reinterpret_cast<LPARAM>(hIcon));
	return *this;
}

const StatusBar::Part& StatusBar::Part::setText(std::wstring_view text) const
{
	SendMessageW(_hSb, SB_SETTEXT,
		MAKEWPARAM(MAKEWORD(_index, 0), 0), reinterpret_cast<LPARAM>(text.data()));
	return *this;
}

std::wstring StatusBar::Part::text() const
{
	UINT len = LOWORD(SendMessageW(_hSb, SB_GETTEXTLENGTHW, _index, 0));
	if (!len) return {};

	std::wstring buf(len + 1, L'\0');
	SendMessageW(_hSb, SB_GETTEXTW, _index, reinterpret_cast<LPARAM>(buf.data()));
	str::trimNulls(buf);
	return buf;
}


StatusBar::Part StatusBar::PartCollection::_addPart(PartData partData, std::wstring_view text)
{
	_partsData.emplace_back(partData);
	_rightEdges.emplace_back(0);
	_resizeToParent(_parentWidth());
	
	UINT numParts = static_cast<UINT>(_partsData.size());
	Part newPart{_pSb, static_cast<int>(numParts) - 1};
	if (!text.empty()) newPart.setText(text);
	return newPart;
}

UINT StatusBar::PartCollection::_parentWidth() const
{
	static int cx = 0; // cache, since parts are intended to be added during window creation only
	if (!cx) {
		RECT rc{};
		GetClientRect(GetParent(_pSb->hWnd()), &rc);
		cx = rc.right;
	}
	return cx;
}

void StatusBar::PartCollection::_resizeToParent(UINT cxParent)
{
	SendMessageW(_pSb->hWnd(), WM_SIZE, 0, 0); // tell statusbar to fit parent

	// Find the space to be divided among variable-width parts,
	// and total weight of variable-width parts.
	UINT totalWeight = 0;
	int cxVariable = cxParent;
	for (const PartData& onePart : _partsData) {
		if (!onePart.resizeWeight) { // fixed-width?
			cxVariable -= onePart.sizePixels;
		} else {
			totalWeight += onePart.resizeWeight;
		}
	}

	// Fill right edges array with the right edge of each part.
	int cxTotal = cxParent;
	for (size_t i = _partsData.size(); i-- > 0; ) {
		_rightEdges[i] = cxTotal;
		cxTotal -= (!_partsData[i].resizeWeight) ? // fixed-width?
			_partsData[i].sizePixels :
			static_cast<int>( (cxVariable / totalWeight) * _partsData[i].resizeWeight );
	}
	SendMessageW(_pSb->hWnd(), SB_SETPARTS,
		_rightEdges.size(), reinterpret_cast<LPARAM>(_rightEdges.data()));
}


const StatusBar& StatusBar::create(Dialog* parent, WORD ctrlId)
{
	HINSTANCE hInst = reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(parent->hWnd(), GWLP_HINSTANCE));

	DWORD parentStyle = static_cast<DWORD>(GetWindowLongPtrW(parent->hWnd(), GWL_STYLE));
	bool isParentResizable = (parentStyle & WS_MAXIMIZEBOX) != 0
		|| (parentStyle & WS_SIZEBOX) != 0;
	DWORD style = WS_CHILD | WS_VISIBLE | (isParentResizable ? SBARS_SIZEGRIP : 0);

	HWND hSb = CreateWindowExW(0, STATUSCLASSNAMEW, nullptr, style,
		0, 0, 0, 0, parent->hWnd(), reinterpret_cast<HMENU>(ctrlId), hInst, nullptr);
	if (!hSb) [[unlikely]] {
		throw std::system_error(GetLastError(), std::system_category(), "CreateWindowEx");
	}

	*_hWndPtr() = hSb;
	return *this;
}

const StatusBar& StatusBar::resizeToParent(UINT cxParent)
{
	parts._resizeToParent(cxParent);
	return *this;
}
