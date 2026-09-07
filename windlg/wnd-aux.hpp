#pragma once
#include "wnd.hpp"
#include <CommCtrl.h>

namespace wd {

	/// Adjusts pixel values according to current system DPI.
	namespace dpi {
		[[nodiscard]] int x(int xVal) noexcept;
		[[nodiscard]] int y(int yVal) noexcept;
		[[nodiscard]] inline POINT pt(int xVal, int yVal) noexcept { return {.x = x(xVal), .y = y(yVal)}; }
		[[nodiscard]] inline POINT pt(POINT ptVal) noexcept        { return pt(ptVal.x, ptVal.y); }
		[[nodiscard]] inline SIZE  sz(int xVal, int yVal) noexcept { return {.cx = x(xVal), .cy = y(yVal)}; }
		[[nodiscard]] inline SIZE  sz(SIZE szVal) noexcept         { return sz(szVal.cx, szVal.cy); }
	}

	// Layout behavior when parent window resizes.
	enum class Lay : BYTE {
		hold_hold, hold_move, hold_resz,
		move_hold, move_move, move_resz,
		resz_hold, resz_move, resz_resz,
	};

	/// Automates layout behavior when parent window resizes.
	class Layout final {
	public:
		explicit Layout(const BaseContainer *pParent, size_t numCtrlsReserve = 0);
		Layout& add(WORD ctrlId, Lay layoutBehavior) { return add({ctrlId}, layoutBehavior); }
		Layout& add(std::initializer_list<WORD> ctrlIds, Lay layoutBehavior);
		void rearrange(WORD req, SIZE sz) const noexcept;
	private:
		struct Child final {
			HWND hCtrl;
			Lay  lay;
			RECT rcOrig;
		};
		const BaseContainer *_pParent;
		std::vector<Child> _children;
		SIZE _szOrig; // original size of parent client area
	};

	class MenuSub;

	/// Wraps HMENU.
	class Menu {
	public:
		virtual ~Menu() = default;
		constexpr explicit Menu(HMENU hMenu) noexcept : _hMenu{hMenu} { }

		[[nodiscard]] constexpr HMENU hmenu() const noexcept  { return _hMenu; }

		const Menu& add(StrView text, WORD cmdId) const;
		const Menu& add_sep() const;
		[[nodiscard]] MenuSub add_sub(StrView text) const;
		const Menu& enable_cmd(bool doEnable, WORD cmdId) const noexcept;
		const Menu& enable_cmds(bool doEnable, std::initializer_list<WORD> cmdIds) const noexcept;
		[[nodiscard]] WORD item_cmd(int index) const noexcept { return static_cast<WORD>(GetMenuItemID(_hMenu, index)); }
		const Menu& set_default_cmd(WORD cmdId) const;
		const Menu& set_cmd_text(WORD cmdId, StrView text) const;
		const Menu& show_at_point(POINT pos, HWND hOwner, HWND hChildCoordsRelativeTo) const;
		Menu sub_menu(UINT index) const                       { return Menu{GetSubMenu(_hMenu, index)}; }
	protected:
		HMENU _hMenu;
	};

	/// Manages an HMENU resource.
	class MenuResource final : public Menu {
	public:
		~MenuResource() noexcept { destroy(); }
		constexpr MenuResource() noexcept : Menu{nullptr} { }

		constexpr MenuResource(MenuResource &&other) noexcept : Menu{other._hMenu} { other._hMenu = nullptr; }
		MenuResource& operator=(MenuResource &&other) noexcept;

		MenuResource& destroy() noexcept;
		MenuResource& create_popup(); // Calls CreatePopupMenu() to create a popup/context menu.
		MenuResource& create_toplevel(); // Calls CreateMenu() to create a top level menu for a window.
		[[nodiscard]] HMENU leak() noexcept;
		MenuResource& load_from_resource(HINSTANCE hInst, WORD menuId); // Calls LoadMenu().
		MenuResource& load_from_resource(HWND hParent, WORD menuId);    // Calls LoadMenu().
	};

	/// Temporary object for a submenu being added.
	class MenuSub final {
	public:
		constexpr MenuSub(HMENU hMenu0, HMENU hMenu1, HMENU hMenu2) noexcept :
			_hMenu0{hMenu0}, _hMenu1{hMenu1}, _hMenu2{hMenu2} { }

		const MenuSub& add(StrView text, WORD cmdId) const;
		const MenuSub& add_sep() const;
		[[nodiscard]] MenuSub add_sub(StrView text) const;
		[[nodiscard]] constexpr MenuSub up() const noexcept { return _hMenu2 ? MenuSub{_hMenu0, _hMenu1, nullptr} : MenuSub{_hMenu0, nullptr, nullptr}; }
	private:
		[[nodiscard]] Menu _current() const noexcept;
		HMENU _hMenu0, _hMenu1, _hMenu2; // up to 3 levels
	};

	/// Manages an HIMAGELIST.
	class ImageList final {
	public:
		~ImageList() noexcept { destroy(); }
		constexpr ImageList() noexcept
			: _hIL{nullptr}, _resolution{0} { }

		const ImageList& add_file_ext(wd::StrView fileExt) const     { return add_file_ext({fileExt}); }
		const ImageList& add_file_ext(std::initializer_list<wd::StrView> fileExts) const;
		const ImageList& add_resource(WORD iconId) const             { return add_resource({iconId}); }
		const ImageList& add_resource(std::initializer_list<WORD> iconIds) const;
		[[nodiscard]] int count() const noexcept                     { return ImageList_GetImageCount(_hIL); }
		ImageList& create16()                                        { return _raw_create(16); } // Calls ImageList_Create().
		ImageList& create32()                                        { return _raw_create(32); } // Calls ImageList_Create().
		ImageList& destroy() noexcept;
		[[nodiscard]] constexpr HIMAGELIST himglist() const noexcept { return _hIL; }
	private:
		ImageList& _raw_create(int resolution);

		HIMAGELIST _hIL;
		int _resolution;
	};

}
