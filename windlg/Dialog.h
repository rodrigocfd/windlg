#pragma once
#include <functional>
#include <optional>
#include <string>
#include <vector>
#include <Windows.h>
#include <oleidl.h>
#include "Com.h"
#include "Window.h"

namespace lib {

// Base to all dialog windows.
class Dialog : public Window {
public:
	enum class Horz { None, Repos, Resize }; // Horizontal layout behavior for child controls.
	enum class Vert { None, Repos, Resize }; // Vertical layout behavior for child controls.

private:
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

		// Adds one or more controls to the automatic layout resizing and repositioning.
		const Facilities& layout(Horz horz, Vert vert, std::initializer_list<WORD> ctrlIds) const;

		// Calls TaskDialogIndirect(); returns IDOK, IDCANCEL, etc.
		int msgBox(std::wstring_view title, std::optional<std::wstring_view> mainInstruction,
			std::wstring_view body, int tdcbfButtons, LPWSTR tdIcon) const;

		// Calls RegisterDragDrop() to enable onDropTarget() callback. Don't forget to instantiate lib::ComOle.
		const Facilities& registerDragDrop() const;

		// Creates a new, detached thread and runs the function. Catches uncaught exceptions.
		void runDetachedThread(std::function<void()> callback) const;

		// Blocks the current thread and runs the function in the original UI thread. Catches uncaught exceptions.
		void runUiThread(std::function<void()> callback) const;

		[[nodiscard]] std::optional<std::wstring> showOpenFile(std::initializer_list<std::pair<std::wstring_view, std::wstring_view>> namesExts) const { return _showOpenSave(true, false, namesExts); }
		[[nodiscard]] std::optional<std::vector<std::wstring>> showOpenFiles(std::initializer_list<std::pair<std::wstring_view, std::wstring_view>> namesExts) const;
		[[nodiscard]] std::optional<std::wstring> showSaveFile(std::initializer_list<std::pair<std::wstring_view, std::wstring_view>> namesExts) const { return _showOpenSave(false, false, namesExts); }
		[[nodiscard]] std::optional<std::wstring> showOpenFolder() const { return _showOpenSave(true, true, {}); }

	private:
		[[nodiscard]] std::optional<std::wstring> _showOpenSave(bool isOpen, bool isFolder,
			std::initializer_list<std::pair<std::wstring_view, std::wstring_view>> namesExts) const;

		Dialog* _pDlg = nullptr; // assumes Dialog is immovable
		friend Dialog;
	};

	class DropTarget final : public IDropTarget {
	public:
		constexpr DropTarget(Dialog* pDlg) : _pDlg{pDlg} { }
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
		[[nodiscard]] std::vector<std::wstring> _getDropped(HDROP hDrop) const;

		Dialog* _pDlg = nullptr; // assumes Dialog is immovable
		LONG _refCount = 1;
	};

	class Layout final {
	public:
		constexpr Layout(Dialog* pDlg) : _pDlg{pDlg} { }
		Layout(const Layout&) = delete;
		Layout(Layout&&) = delete;
		Layout& operator=(const Layout&) = delete;
		Layout& operator=(Layout&&) = delete;

		void add(Horz horz, Vert vert, std::initializer_list<WORD> ctrlIds);
		void processMsgs(UINT uMsg, WPARAM wp, LPARAM lp) const;

	private:
		struct ChildInfo final {
			HWND hCtrl = nullptr;
			Horz horz = Horz::None;
			Vert vert = Vert::None;
			RECT rcOrig{};
		};

		Dialog* _pDlg = nullptr; // assumes Dialog is immovable
		std::vector<ChildInfo> _ctrls;
		SIZE _szParentOrig{};
	};

public:
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
	void _runFromOtherThread(LPARAM lp) const;
	Window::_hWndPtr;

	ComPtr<DropTarget> _pDropTarget;
	Layout _layout{this};
};

}
