#pragma once
#include "Dialog.h"

namespace lib {

// Base to the main application window.
class DialogMain : public Dialog {
public:
	virtual ~DialogMain() { }

	constexpr DialogMain() = default;
	DialogMain(const DialogMain&) = delete;
	DialogMain(DialogMain&&) = delete;
	DialogMain& operator=(const DialogMain&) = delete;
	DialogMain& operator=(DialogMain&&) = delete;

	// Creates the main dialog, and returns only after the dialog is closed.
	int runMain(HINSTANCE hInst, WORD dlgId, int cmdShow, WORD iconId = 0, WORD accelTblId = 0) const;

private:
	static void _SetTimerSafety();
	HACCEL _loadAccelTbl(HINSTANCE hInst, WORD accelTblId) const;
	void _setIcon(HINSTANCE hInst, WORD iconId) const;
	int _mainLoop(HACCEL hAccel) const;
	Dialog::_DlgProc;
};

}
