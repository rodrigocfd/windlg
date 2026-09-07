#pragma once
#include <functional>
#include "string.hpp"

namespace wd {

	/// An accelerator table entry, passed to run_main_dialog().
	struct Acc final {
		// A key modifier for an accelerator.
		enum class Key : BYTE {
			none           = FVIRTKEY,
			ctrl           = FVIRTKEY | FCONTROL,
			shift          = FVIRTKEY | FSHIFT,
			alt            = FVIRTKEY | FALT,
			ctrl_shift     = FVIRTKEY | FCONTROL | FSHIFT,
			ctrl_alt       = FVIRTKEY | FCONTROL | FALT,
			shift_alt      = FVIRTKEY | FSHIFT | FALT,
			ctrl_shift_alt = FVIRTKEY | FCONTROL | FSHIFT | FALT,
		};

		Key modifiers;
		WORD vkey;
		WORD cmdId;
	};

	class BaseContainer;
	class BaseDialog;

	int run_main_dialog(HINSTANCE hInst, int cmdShow, BaseDialog &dlg, std::initializer_list<Acc> accelTbl = {}); // Runs the main window, blocking until it's closed.
	void run_modal_dialog(BaseContainer *pOwner, BaseDialog &dlg); // Runs the modal window, blocking until it's closed.
	void run_ui_thread(BaseContainer *pWnd, std::function<void()> f); // Runs the function in the original UI thread, synchronously. Useful to update the UI from a spawned thread.

	/// Base class to custom container windows.
	class BaseContainer {
	public:
		virtual ~BaseContainer() = default;
		[[nodiscard]] virtual constexpr HWND hwnd() const noexcept = 0;
	};

	/// Base class to build a dialog.
	class BaseDialog : public BaseContainer {
	public:
		virtual ~BaseDialog() = default;
		constexpr explicit BaseDialog(WORD dlgId, WORD iconId = 0) noexcept
			: _dlgId{dlgId}, _iconId{iconId}, _hWnd{nullptr}, _isModal{false} { }

		BaseDialog(const BaseDialog&) = delete;
		BaseDialog& operator=(const BaseDialog&) = delete; // non-copyable
		BaseDialog(BaseDialog&&) = delete;
		BaseDialog& operator=(BaseDialog&&) = delete; // non-movable

		[[nodiscard]] constexpr HWND hwnd() const noexcept override { return _hWnd; }
		void set_text(StrView text) const noexcept                  { SetWindowTextW(hwnd(), text.c_str()); }
		[[nodiscard]] std::wstring text() const;

	protected:
		virtual void on_close()                           { _isModal ? EndDialog(hwnd(), 0) : DestroyWindow(hwnd()); }
		virtual bool on_command(WORD id, WORD code)       { (void)id; (void)code; return false; }
		virtual void on_destroy()                         { if (!_isModal) PostQuitMessage(0); }
		virtual bool on_get_min_max_info(MINMAXINFO &mmi) { (void)mmi; return false; }
		virtual bool on_init_dialog()                     { return true; }
		virtual void on_init_menu_popup(HMENU hMenu)      { (void)hMenu; }
		virtual bool on_notify(NMHDR &nm)                 { (void)nm; return false; }
		virtual void on_size(WORD req, SIZE sz)           { (void)req; (void)sz; }
		virtual bool on_sizing(WORD edge, RECT &rc)       { (void)edge; (void)rc; return false; }

	private:
		static INT_PTR CALLBACK _raw_dlg_proc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
		WORD _dlgId, _iconId;
		HWND _hWnd; // set in WM_INITDIALOG
		bool _isModal;

		friend int run_main_dialog(HINSTANCE, int, BaseDialog&, std::initializer_list<Acc>);
		friend void run_modal_dialog(BaseContainer*, BaseDialog&);
	};

	/// Base class to build a custom child control.
	class BaseControl : public BaseContainer {
	public:
		virtual ~BaseControl() = default;
		constexpr explicit BaseControl(const BaseContainer *pParent, WORD ctrlId = 0) noexcept
			: _pParent{pParent}, _hWnd{nullptr}, _ctrlId{ctrlId} { }

		BaseControl(const BaseControl&) = delete;
		BaseControl& operator=(const BaseControl&) = delete; // non-copyable
		BaseControl(BaseControl&&) = delete;
		BaseControl& operator=(BaseControl&&) = delete; // non-movable

		/// Used when creating the control.
		struct {
			const WCHAR *className = nullptr;
			DWORD classStyle = CS_DBLCLKS;
			HCURSOR hCursor = nullptr;
			BYTE bgColor = COLOR_WINDOW;
			DWORD wndStyle = WS_CHILD | WS_TABSTOP | WS_GROUP | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
			DWORD wndExStyle = WS_EX_LEFT;
			bool border = true;
		} setup{};

		[[nodiscard]] constexpr WORD ctrl_id() const noexcept                { return _ctrlId; }
		[[nodiscard]] constexpr HWND hwnd() const noexcept override          { return _hWnd; }
		[[nodiscard]] constexpr const BaseContainer* parent() const noexcept { return _pParent; }
		void create(int x, int y, int cx, int cy);
		void create(POINT pt, SIZE sz)                                       { create(pt.x, pt.y, sz.cx, sz.cy); }

	protected:
		virtual LRESULT wnd_proc(UINT msg, WPARAM wp, LPARAM lp) = 0;

	private:
		static LRESULT CALLBACK _raw_wnd_proc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
		const BaseContainer *_pParent;
		HWND _hWnd; // set in WM_CREATE
		WORD _ctrlId;
	};

}
