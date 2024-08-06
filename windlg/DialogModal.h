#pragma once
#include "Dialog.h"

namespace lib {

// Base to modal windows.
class DialogModal : public Dialog {
public:
	virtual ~DialogModal() { }

	constexpr DialogModal() = default;
	DialogModal(const DialogModal&) = delete;
	DialogModal(DialogModal&&) = delete;
	DialogModal& operator=(const DialogModal&) = delete;
	DialogModal& operator=(DialogModal&&) = delete;

	// Displays the modal dialog, and returns only after the dialog is closed.
	INT_PTR showModal(const Dialog* parent, WORD dlgId) const;

private:
	Dialog::_DlgProc;
	Dialog::_Lippincott;
};

}
