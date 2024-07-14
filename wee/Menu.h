#pragma once
#include <initializer_list>
#include <utility>
#include <Windows.h>

namespace lib {

// Simple HMENU wrapper.
class Menu final {
private:
	HMENU _hMenu = nullptr;

public:
	constexpr Menu() = default;
	constexpr Menu(const Menu&) = default;
	Menu(Menu&& other) noexcept { operator=(std::forward<Menu>(other)); }
	constexpr Menu& operator=(const Menu&) = default;
	Menu& operator=(Menu&& other) noexcept;

	constexpr explicit Menu(HMENU hMenu) : _hMenu{hMenu} { }
	Menu(HINSTANCE hInst, WORD menuId);

	[[nodiscard]] constexpr HMENU hMenu() const { return _hMenu; }
	void enableItemsByCmd(std::initializer_list<WORD> cmdIds, bool doEnable = true);
	void enableItemsByPos(std::initializer_list<UINT> poss, bool doEnable = true);
	void setDefaultItemByCmd(WORD cmdId) const;
	void setDefaultItemByPos(UINT pos) const;
	void showAtPoint(int x, int y, HWND hParent, HWND hWndCoordsRelativeTo) const;
	[[nodiscard]] Menu subMenu(UINT pos) const;
};

}
