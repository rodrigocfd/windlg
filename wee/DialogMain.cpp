#include <system_error>
#include <Windows.h>
#include <VersionHelpers.h>
#include <CommCtrl.h>
#pragma comment(lib, "Comctl32.lib")
#include "DialogMain.h"
using namespace lib;

int DialogMain::runMain(HINSTANCE hInst, WORD dlgId, int cmdShow, WORD iconId, WORD accelTblId) const
{
	InitCommonControls();
	_SetTimerSafety();
	
	if (!CreateDialogParamW(hInst, MAKEINTRESOURCEW(dlgId),
			nullptr, Dialog::_DlgProc, reinterpret_cast<LPARAM>(this))) [[unlikely]] {
		throw std::system_error(GetLastError(), std::system_category(), "CreateDialogParam failed");
	}

	HACCEL hAccel = _loadAccelTbl(hInst, accelTblId);
	_setIcon(hInst, iconId);
	ShowWindow(hWnd(), cmdShow);
	int ret = _mainLoop(hAccel);
	return ret;
}

void DialogMain::_SetTimerSafety()
{
	if (IsWindows8OrGreater()) {
		BOOL bVal = FALSE;
		if (!SetUserObjectInformationW(GetCurrentProcess(),
				UOI_TIMERPROC_EXCEPTION_SUPPRESSION, &bVal, sizeof(bVal))) [[unlikely]] { // SetTimer() safety
			throw std::system_error(GetLastError(), std::system_category(), "SetUserObjectInformation failed");
		}
	}
}

HACCEL DialogMain::_loadAccelTbl(HINSTANCE hInst, WORD accelTblId) const
{
	if (!accelTblId) {
		return nullptr;
	} else {
		HACCEL hAccel = LoadAcceleratorsW(hInst, MAKEINTRESOURCE(accelTblId));
		if (!hAccel) [[unlikely]] {
			throw std::system_error(GetLastError(), std::system_category(), "LoadAccelerators failed");
		}
		return hAccel;
	}
}

void DialogMain::_setIcon(HINSTANCE hInst, WORD iconId) const
{
	if (iconId) {
		SendMessageW(hWnd(), WM_SETICON, ICON_SMALL,
			reinterpret_cast<LPARAM>(reinterpret_cast<HICON>(LoadImageW(hInst,
				MAKEINTRESOURCEW(iconId), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR))));
		SendMessageW(hWnd(), WM_SETICON, ICON_BIG,
			reinterpret_cast<LPARAM>(reinterpret_cast<HICON>(LoadImageW(hInst,
				MAKEINTRESOURCEW(iconId), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR))));
	}
}

int DialogMain::_mainLoop(HACCEL hAccel) const
{
	MSG msg{};
	BOOL ret = FALSE;

	while ((ret = GetMessageW(&msg, nullptr, 0, 0)) != 0) {
		if (ret == -1) [[unlikely]] {
			throw std::system_error(GetLastError(), std::system_category(), "GetMessage failed");
		}

		if (hAccel && TranslateAcceleratorW(hWnd(), hAccel, &msg)) continue;
		if (IsDialogMessageW(hWnd(), &msg)) continue;
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	return static_cast<int>(msg.wParam); // this can be used as program return value
}
