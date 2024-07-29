#pragma once
#include <functional>
#include <optional>
#include <string>
#include <vector>
#include "Window.h"

namespace lib {

// Base to all dialog windows.
class Dialog : public Window {
public:
	class DialogFacilities final {
	private:
		friend Dialog;
		Dialog* _pDlg = nullptr; // assumes Dialog is immovable
		constexpr DialogFacilities(Dialog *pDlg) : _pDlg{pDlg} { }

	public:
		DialogFacilities() = delete;
		DialogFacilities(const DialogFacilities&) = delete;
		DialogFacilities(DialogFacilities&&) = delete;
		DialogFacilities& operator=(const DialogFacilities&) = delete;
		DialogFacilities& operator=(DialogFacilities&&) = delete;

		// Calls DragQueryFile() for each file, then DragFinish().
		[[nodiscard]] std::vector<std::wstring> droppedFiles(HDROP hDrop) const;

		// Calls EnableWindow() for each child control.
		void enable(std::initializer_list<WORD> ctrlIds, bool doEnable = true) const;

		// Creates a new, detached thread and runs the function. Catches uncaught exceptions.
		void runDetachedThread(std::function<void()> callback) const;

		// Blocks the current thread and runs the function in the original UI thread. Catches uncaught exceptions.
		void runUiThread(std::function<void()> callback) const;

		[[nodiscard]] std::optional<std::wstring> showOpenFile(std::initializer_list<std::pair<std::wstring_view, std::wstring_view>> namesExts) const { return _showOpenSave(true, false, namesExts); }
		[[nodiscard]] std::optional<std::vector<std::wstring>> showOpenFiles(std::initializer_list<std::pair<std::wstring_view, std::wstring_view>> namesExts) const;
		[[nodiscard]] std::optional<std::wstring> showSaveFile(std::initializer_list<std::pair<std::wstring_view, std::wstring_view>> namesExts) const { return _showOpenSave(false, false, namesExts); }
		[[nodiscard]] std::optional<std::wstring> showOpenFolder() const { return _showOpenSave(true, true, {}); }
		
		// Calls TaskDialogIndirect(); returns IDOK, IDCANCEL, etc.
		int msgBox(std::wstring_view title, std::wstring_view mainInstruction,
			std::wstring_view body, int tdcbfButtons, LPWSTR tdIcon) const;

	private:
		[[nodiscard]] std::optional<std::wstring> _showOpenSave(bool isOpen, bool isFolder,
			std::initializer_list<std::pair<std::wstring_view, std::wstring_view>> namesExts) const;
	};

	virtual ~Dialog() { }

	constexpr Dialog() = default;
	Dialog(const Dialog&) = delete;
	Dialog(Dialog&&) = delete;
	Dialog& operator=(const Dialog&) = delete;
	Dialog& operator=(Dialog&&) = delete;

protected:
	// Dialog facilities.
	DialogFacilities dlg{this};

	virtual INT_PTR dlgProc(UINT uMsg, WPARAM wp, LPARAM lp) = 0; // to be overriden in user class
	static INT_PTR CALLBACK _DlgProc(HWND hDlg, UINT uMsg, WPARAM wp, LPARAM lp);

private:
	void _runFromOtherThread(LPARAM lp) const;
protected:
	static void _Lippincott();
private:
	Window::_hWndPtr;
};

}
