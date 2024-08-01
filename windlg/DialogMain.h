#pragma once
#include "Dialog.h"

namespace lib {

class DialogMain;

// Creates the main dialog, and returns only after the dialog is closed. Catches uncaught exceptions.
int runMain(DialogMain& dlgObj, HINSTANCE hInst, WORD dlgId, int cmdShow, WORD iconId = 0, WORD accelTblId = 0);

// Base to the main application window.
class DialogMain : public Dialog {
public:
	virtual ~DialogMain() { }

	constexpr DialogMain() = default;
	DialogMain(const DialogMain&) = delete;
	DialogMain(DialogMain&&) = delete;
	DialogMain& operator=(const DialogMain&) = delete;
	DialogMain& operator=(DialogMain&&) = delete;

private:
	Dialog::_DlgProc;
	Dialog::_Lippincott;
	friend int runMain(DialogMain&, HINSTANCE, WORD, int, WORD, WORD);
};

}
