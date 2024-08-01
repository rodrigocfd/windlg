#pragma once
#include <functional>
#include <optional>
#include <string>
#include <vector>
#include <Windows.h>
#include <oleidl.h>
#include "Window.h"

namespace lib {

// Base to all dialog windows.
class Dialog : public Window {
public:
	class Facilities final {
	private:
		constexpr Facilities(Dialog* pDlg) : _pDlg{pDlg} { }

	public:
		Facilities() = delete;
		Facilities(const Facilities&) = delete;
		Facilities(Facilities&&) = delete;
		Facilities& operator=(const Facilities&) = delete;
		Facilities& operator=(Facilities&&) = delete;

		// Calls EnableWindow() for each child control.
		void enable(std::initializer_list<WORD> ctrlIds, bool doEnable = true) const;

		// Calls RegisterDragDrop() to enable onDropTarget() callback. Don't forget to instantiate lib::ComOle.
		void registerDragDrop() const;

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

		Dialog* _pDlg = nullptr; // assumes Dialog is immovable
		friend Dialog;
	};

	virtual ~Dialog() { }

	constexpr Dialog() = default;
	Dialog(const Dialog&) = delete;
	Dialog(Dialog&&) = delete;
	Dialog& operator=(const Dialog&) = delete;
	Dialog& operator=(Dialog&&) = delete;

protected:
	// Dialog facilities.
	Facilities dlg{this};

	virtual INT_PTR dlgProc(UINT uMsg, WPARAM wp, LPARAM lp) = 0; // to be overriden in user class
	virtual void onDropTarget(const std::vector<std::wstring>& files) { }
	static INT_PTR CALLBACK _DlgProc(HWND hDlg, UINT uMsg, WPARAM wp, LPARAM lp);
	static void _Lippincott();

private:
	class DropTarget final : public IDropTarget {
	private:
		constexpr DropTarget(Dialog* pDlg) : _pDlg{pDlg} { }

	public:
		DropTarget(const DropTarget&) = delete;
		DropTarget(DropTarget&&) = delete;
		DropTarget& operator=(const DropTarget&) = delete;
		DropTarget& operator=(DropTarget&&) = delete;

		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override;
		ULONG STDMETHODCALLTYPE AddRef() override;
		ULONG STDMETHODCALLTYPE Release() override;

		HRESULT STDMETHODCALLTYPE DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override;
		HRESULT STDMETHODCALLTYPE DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override;
		HRESULT STDMETHODCALLTYPE DragLeave() override { return S_OK; }
		HRESULT STDMETHODCALLTYPE Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override;

	private:
		std::vector<std::wstring> _getDropped(HDROP hDrop) const;

		Dialog* _pDlg = nullptr; // assumes Dialog is immovable
		LONG _refCount = 0;
		friend Dialog;
	};

	void _runFromOtherThread(LPARAM lp) const;
	Window::_hWndPtr;

	DropTarget _dropTarget{this};
	bool _usingDropTarget = false;
};

}
