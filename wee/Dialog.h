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
	class SysDlgs final {
	private:
		friend Dialog;
		Dialog* _pDlg = nullptr;
		constexpr SysDlgs(Dialog *pDlg) : _pDlg{pDlg} { }
	public:
		SysDlgs() = delete;
		SysDlgs(const SysDlgs&) = delete;
		SysDlgs(SysDlgs&&) = delete;
		SysDlgs& operator=(const SysDlgs&) = delete;
		SysDlgs& operator=(SysDlgs&&) = delete;

		[[nodiscard]] std::optional<std::wstring> openFile(std::initializer_list<std::pair<std::wstring_view, std::wstring_view>> namesExts) const { return _openSave(true, false, namesExts); }
		[[nodiscard]] std::optional<std::vector<std::wstring>> openFiles(std::initializer_list<std::pair<std::wstring_view, std::wstring_view>> namesExts) const;
		[[nodiscard]] std::optional<std::wstring> saveFile(std::initializer_list<std::pair<std::wstring_view, std::wstring_view>> namesExts) const { return _openSave(false, false, namesExts); }
		[[nodiscard]] std::optional<std::wstring> openFolder() const { return _openSave(true, true, {}); }
		
		// Calls TaskDialogIndirect(); returns IDOK, IDCANCEL, etc.
		int msgBox(std::wstring_view title, std::wstring_view mainInstruction,
			std::wstring_view body, int tdcbfButtons, LPWSTR tdIcon) const;

	private:
		[[nodiscard]] std::optional<std::wstring> _openSave(bool isOpen, bool isFolder,
			std::initializer_list<std::pair<std::wstring_view, std::wstring_view>> namesExts) const;
	};

	virtual ~Dialog() { }

	constexpr Dialog() = default;
	Dialog(const Dialog&) = delete;
	Dialog(Dialog&&) = delete;
	Dialog& operator=(const Dialog&) = delete;
	Dialog& operator=(Dialog&&) = delete;

protected:
	// Exposes standard system modal dialogs.
	SysDlgs sys{this};

	virtual INT_PTR dlgProc(UINT uMsg, WPARAM wp, LPARAM lp) = 0; // to be overriden in user class
	static INT_PTR CALLBACK _DlgProc(HWND hDlg, UINT uMsg, WPARAM wp, LPARAM lp);

	// Calls DragQueryFile() for each file, then DragFinish().
	[[nodiscard]] std::vector<std::wstring> droppedFiles(HDROP hDrop) const;

	// Calls EnableWindow() for each child control.
	void enable(std::initializer_list<WORD> ctrlIds, bool doEnable = true) const;

	// Blocks the current thread and runs the function in the original UI thread.
	void runUiThread(std::function<void()> callback) const;

private:
	void _processUiThread(LPARAM lp) const;
	Window::_hWndPtr;
};

}
