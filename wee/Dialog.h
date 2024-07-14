#pragma once
#include <optional>
#include <string>
#include <vector>
#include "Window.h"

namespace lib {

// Base to all dialog windows.
class Dialog : public Window {
public:
	virtual ~Dialog() { }

	constexpr Dialog() = default;
	Dialog(const Dialog&) = delete;
	Dialog(Dialog&&) = delete;
	Dialog& operator=(const Dialog&) = delete;
	Dialog& operator=(Dialog&&) = delete;

protected:
	virtual INT_PTR dlgProc(UINT uMsg, WPARAM wp, LPARAM lp) = 0; // to be overriden in user class
	static INT_PTR CALLBACK _DlgProc(HWND hDlg, UINT uMsg, WPARAM wp, LPARAM lp);

	// Calls DragQueryFile() for each file, then DragFinish().
	[[nodiscard]] std::vector<std::wstring> droppedFiles(HDROP hDrop) const;

	[[nodiscard]] std::optional<std::wstring> showOpenFile(std::initializer_list<std::pair<std::wstring_view, std::wstring_view>> namesExts) const { return _showOpenSave(true, false, namesExts); }
	[[nodiscard]] std::optional<std::vector<std::wstring>> showOpenFiles(std::initializer_list<std::pair<std::wstring_view, std::wstring_view>> namesExts) const;
	[[nodiscard]] std::optional<std::wstring> showSaveFile(std::initializer_list<std::pair<std::wstring_view, std::wstring_view>> namesExts) const { return _showOpenSave(false, false, namesExts); }
	[[nodiscard]] std::optional<std::wstring> showOpenFolder() const { return _showOpenSave(true, true, {}); }
	[[nodiscard]] std::optional<std::wstring> _showOpenSave(bool isOpen, bool isFolder,
		std::initializer_list<std::pair<std::wstring_view, std::wstring_view>> namesExts) const;

	// Calls TaskDialogIndirect(); returns IDOK, IDCANCEL, etc.
	int msgBox(std::wstring_view title, std::wstring_view mainInstruction,
		std::wstring_view body, int tdcbfButtons, LPWSTR tdIcon) const;

private:
	Window::_hWndPtr;
};

}
