#include "wnd.hpp"
#include "win-error.hpp"
#include <VersionHelpers.h>
#include <CommCtrl.h>
using namespace wd;

#ifdef _MSC_VER
#pragma comment(lib, "comctl32.lib")
#endif

struct ThreadPack final {
	std::function<void()> f;
};
#define WM_THREAD WM_APP + 0x3fff

static void main_init_routines() {
	InitCommonControls();
	if (IsWindows8OrGreater()) {
		HANDLE hProcess = GetCurrentProcess();
		BOOL bVal = FALSE;
		BOOL ok = SetUserObjectInformationW(hProcess, UOI_TIMERPROC_EXCEPTION_SUPPRESSION, &bVal, sizeof(BOOL));
		if (!ok) [[unlikely]] {
			DWORD err = GetLastError();
			if (err == ERROR_INVALID_PARAMETER) {
				// Do nothing: Wine doesn't support SetUserObjectInformation for now.
				// https://bugs.winehq.org/show_bug.cgi?id=54951
			}
			throw WinErr{err, L"SetUserObjectInformation failed."};
		}
	}
}

static void load_main_dlg_icon(HINSTANCE hInst, HWND hWnd, WORD iconId) {
	if (iconId) {
		HANDLE hIcon16 = LoadImageW(hInst, MAKEINTRESOURCEW(iconId), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
		if (!hIcon16) [[unlikely]] {
			throw WinErr{GetLastError(), L"LoadImage 16x16 failed."};
		}

		HANDLE hIcon32 = LoadImageW(hInst, MAKEINTRESOURCEW(iconId), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
		if (!hIcon32) [[unlikely]] {
			throw WinErr{GetLastError(), L"LoadImage 32x32 failed."};
		}

		SendMessageW(hWnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hIcon16));
		SendMessageW(hWnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(hIcon32));
	}
}

static int run_main_loop(HWND hWnd, HACCEL hAccel) {
	bool processDlgMsgs = true; // will inhibit WM_CHAR, set false for text processors and alike
	MSG msg{};
	for (;;) {
		BOOL ret = GetMessageW(&msg, nullptr, 0, 0);
		if (ret == -1) {
			throw WinErr{GetLastError(), L"GetMessage failed."};
		} else if (!ret) {
			// WM_QUIT was sent, gracefully terminate the program, wParam is the program exit code.
			// https://learn.microsoft.com/en-us/windows/win32/winmsg/using-messages-and-message-queues
			break;
		}

		// If a child window, will retrieve its top-level parent.
		// If a top-level, use itself.
		HWND hWndTopLevel = GetAncestor(hWnd, GA_ROOT);
		if (!hWndTopLevel)
			hWndTopLevel = msg.hwnd;

		// If we have an accelerator table, try to translate the message.
		if (hAccel && TranslateAcceleratorW(hWndTopLevel, hAccel, &msg))
			continue;

		// Try to process keyboard actions for child controls.
		if (processDlgMsgs && IsDialogMessageW(hWndTopLevel, &msg))
			continue;

		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	return static_cast<int>(msg.wParam); // can be used as program return value
}

struct AccelTable final {
	~AccelTable() noexcept { if (hAccel) DestroyAcceleratorTable(hAccel); } // cleanup
	HACCEL hAccel = nullptr;
};

int wd::run_main_dialog(HINSTANCE hInst, int cmdShow, BaseDialog &dlg, std::initializer_list<Acc> accelTbl) {
	AccelTable internalAcc{};
	if (!accelTbl.empty()) {
		internalAcc.hAccel = CreateAcceleratorTableW(
			const_cast<ACCEL*>(reinterpret_cast<const ACCEL*>(accelTbl.begin())), static_cast<int>(accelTbl.size()));
		if (!internalAcc.hAccel) [[unlikely]] {
			throw WinErr{GetLastError(), L"CreateAcceleratorTable failed."};
		}
	}

	dlg._isModal = false;
	main_init_routines();
	CreateDialogParamW(hInst, MAKEINTRESOURCEW(dlg._dlgId), // hWnd member is set in WM_INITDIALOG
		nullptr, BaseDialog::_raw_dlg_proc, reinterpret_cast<LPARAM>(&dlg));
	if (!dlg.hwnd()) [[unlikely]] {
		throw WinErr{GetLastError(), L"CreateDialogParam failed."};
	}

	load_main_dlg_icon(hInst, dlg.hwnd(), dlg._iconId);
	ShowWindow(dlg.hwnd(), cmdShow);
	return run_main_loop(dlg.hwnd(), internalAcc.hAccel);
}

void wd::run_modal_dialog(BaseContainer *pOwner, BaseDialog &dlg) {
	dlg._isModal = true;
	INT_PTR ret = DialogBoxParamW(
		reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(pOwner->hwnd(), GWLP_HINSTANCE)),
		MAKEINTRESOURCEW(dlg._dlgId), pOwner->hwnd(), BaseDialog::_raw_dlg_proc,
		reinterpret_cast<LPARAM>(&dlg));
	if (ret == -1) [[unlikely]] {
		throw WinErr{GetLastError(), L"DialogBoxParam failed."};
	}
}

void wd::run_ui_thread(BaseContainer *pWnd, std::function<void()> f) {
	ThreadPack *pPack = new ThreadPack{std::move(f)};
	SendMessageW(pWnd->hwnd(), WM_THREAD, WM_THREAD, reinterpret_cast<LPARAM>(pPack));
}


std::wstring BaseDialog::text() const {
	return _wd_internal::get_wnd_text(hwnd());
}

INT_PTR BaseDialog::_raw_dlg_proc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
	if (msg == WM_INITDIALOG) {
		BaseDialog *pSelf = reinterpret_cast<BaseDialog*>(lp);
		pSelf->_hWnd = hWnd;
		SetWindowLongPtrW(hWnd, DWLP_USER, reinterpret_cast<LONG_PTR>(pSelf));
	}

	if (msg == WM_THREAD && wp == WM_THREAD) {
		ThreadPack *pPack = reinterpret_cast<ThreadPack*>(lp);
		pPack->f();
		delete pPack;
	}

	if (BaseDialog *pSelf = reinterpret_cast<BaseDialog*>(GetWindowLongPtrW(hWnd, DWLP_USER)); pSelf) {
		switch (msg) {
			case WM_CLOSE:         pSelf->on_close(); return TRUE;
			case WM_COMMAND:       return pSelf->on_command(LOWORD(wp), HIWORD(wp));
			case WM_DESTROY:       pSelf->on_destroy(); return TRUE;
			case WM_GETMINMAXINFO: return pSelf->on_get_min_max_info(*reinterpret_cast<MINMAXINFO*>(lp));
			case WM_INITDIALOG:    return pSelf->on_init_dialog();
			case WM_INITMENUPOPUP: pSelf->on_init_menu_popup(reinterpret_cast<HMENU>(wp)); return TRUE;
			case WM_NOTIFY:        return pSelf->on_notify(*reinterpret_cast<NMHDR*>(lp));
			case WM_SIZE:          pSelf->on_size(static_cast<WORD>(wp), {LOWORD(lp), HIWORD(lp)}); return TRUE;
			case WM_SIZING:        return pSelf->on_sizing(static_cast<WORD>(wp), *reinterpret_cast<RECT*>(lp));
		}
	}
	return FALSE;
}


void BaseControl::create(int x, int y, int cx, int cy) {
	HINSTANCE hInst = reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(parent()->hwnd(), GWLP_HINSTANCE));
	WNDCLASSEXW wcx{
		.cbSize = sizeof(WNDCLASSEXW),
		.style = setup.classStyle,
		.lpfnWndProc = _raw_wnd_proc,
		.hInstance = hInst,
		.hCursor = setup.hCursor ? setup.hCursor : LoadCursorW(nullptr, IDC_ARROW),
		.hbrBackground = reinterpret_cast<HBRUSH>(static_cast<UINT_PTR>(setup.bgColor) + 1),
	};

	std::wstring className{};
	if (setup.className) {
		className = setup.className;
	} else {
		className = str::fmt(L"WNDCLASS %x.%x.%x.%x %x.%x.%x %x.%x.%x",
			wcx.style, wcx.lpfnWndProc, wcx.cbClsExtra, wcx.cbWndExtra,
			wcx.hInstance, wcx.hIcon, wcx.hIconSm,
			wcx.hbrBackground, wcx.hIconSm, wcx.lpszMenuName);
	}
	wcx.lpszClassName = className.c_str();

	ATOM atom = RegisterClassExW(&wcx);
	if (!atom) {
		DWORD err = GetLastError();
		if (err == ERROR_CLASS_ALREADY_EXISTS) [[likely]] {
			// https://devblogs.microsoft.com/oldnewthing/20150429-00/?p=44984
			// https://devblogs.microsoft.com/oldnewthing/20041011-00/?p=37603
			// Retrieve atom from existing window class.
			atom = static_cast<ATOM>(GetClassInfoExW(hInst, className.c_str(), &wcx));
			if (!atom) [[unlikely]] {
				throw WinErr{GetLastError(), L"GetClasssInfoEx failed."};
			}
		} else [[unlikely]] {
			throw WinErr{err, L"RegisterClassEx failed."};
		}
	}

	CreateWindowExW(setup.wndExStyle | (setup.border ? WS_EX_CLIENTEDGE : 0),
		className.c_str(), nullptr, setup.wndStyle, x, y, cx, cy, parent()->hwnd(),
		reinterpret_cast<HMENU>(_ctrlId), hInst, reinterpret_cast<LPVOID>(this));
	if (!hwnd()) [[unlikely]] {
		throw WinErr{GetLastError(), L"CreateWindowEx failed."};
	}
}

LRESULT BaseControl::_raw_wnd_proc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
	if (msg == WM_CREATE) {
		BaseControl *pSelf = reinterpret_cast<BaseControl*>(reinterpret_cast<const CREATESTRUCTW*>(lp)->lpCreateParams);
		pSelf->_hWnd = hWnd;
		SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pSelf));
	}

	if (msg == WM_THREAD && wp == WM_THREAD) {
		ThreadPack *pPack = reinterpret_cast<ThreadPack*>(lp);
		pPack->f();
		delete pPack;
	} else if (msg == WM_NCPAINT) {
		_wd_internal::paint_themed_borders(hWnd, wp, lp);
		return 0;
	}

	if (BaseControl *pSelf = reinterpret_cast<BaseControl*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA)); pSelf)
		return pSelf->wnd_proc(msg, wp, lp);
	return DefWindowProcW(hWnd, msg, wp, lp);
}
