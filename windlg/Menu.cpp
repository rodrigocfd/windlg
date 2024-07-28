#include "Menu.h"
using namespace lib;

Menu& Menu::operator=(Menu&& other) noexcept
{
	_hMenu = other._hMenu;
	other._hMenu = nullptr;
	return *this;
}

Menu::Menu(HINSTANCE hInst, WORD menuId)
	: Menu{LoadMenuW(hInst, MAKEINTRESOURCEW(menuId))}
{
}

void Menu::destroy()
{
	DestroyMenu(_hMenu);
	_hMenu = nullptr;
}

void Menu::enableItemsByCmd(std::initializer_list<WORD> cmdIds, bool doEnable) const
{
	for (auto&& cmdId : cmdIds)
		EnableMenuItem(_hMenu, cmdId, MF_BYCOMMAND | (doEnable ? MF_ENABLED : MF_DISABLED));
}

void Menu::enableItemsByPos(std::initializer_list<UINT> poss, bool doEnable) const
{
	for (auto&& pos : poss)
		EnableMenuItem(_hMenu, pos, MF_BYPOSITION | (doEnable ? MF_ENABLED : MF_DISABLED));
}

WORD Menu::idByPos(UINT pos) const
{
	return GetMenuItemID(_hMenu, pos);
}

void Menu::setDefaultItemByCmd(WORD cmdId) const
{
	SetMenuDefaultItem(_hMenu, cmdId, FALSE);
}

void Menu::setDefaultItemByPos(UINT pos) const
{
	SetMenuDefaultItem(_hMenu, pos, TRUE);
}

void Menu::showAtPoint(int x, int y, HWND hParent, HWND hWndCoordsRelativeTo) const
{
	POINT coords = {.x = x, .y = y};
	ClientToScreen(hWndCoordsRelativeTo, &coords);
	SetForegroundWindow(hParent);
	TrackPopupMenu(_hMenu, TPM_LEFTBUTTON, coords.x, coords.y, 0, hParent, nullptr);
	PostMessageW(hParent, WM_NULL, 0, 0); // necessary according to TrackPopupMenu docs
}

Menu Menu::subMenu(UINT pos) const
{
	return Menu{GetSubMenu(_hMenu, pos)};
}
