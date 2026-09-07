#pragma once
#include "com.hpp"
#include "wnd-ctl.hpp"

namespace wd {

	/// Automates OleInitialize and OleUninitialize calls.
	struct OleInit final {
		~OleInit() noexcept { OleUninitialize(); }
		OleInit();
	};

	/// Automates IDropTarget usage.
	class DropTarget final {
	private:
		struct Impl final : public IDropTarget {
			HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject) override;
			ULONG   STDMETHODCALLTYPE AddRef() override;
			ULONG   STDMETHODCALLTYPE Release() override;
			HRESULT STDMETHODCALLTYPE DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect) override;
			HRESULT STDMETHODCALLTYPE DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect) override;
			HRESULT STDMETHODCALLTYPE DragLeave() override { return S_OK; }
			HRESULT STDMETHODCALLTYPE Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect) override;

			std::vector<std::wstring> get_files(IDataObject *pDataObj) const;
			LONG refCount = 1;
			bool proceedDrop = true;
			std::function<bool(const std::vector<std::wstring>&)> on_drag_enter{};
			std::function<void(const std::vector<std::wstring>&)> on_drop{};
		};

	public:
		~DropTarget() noexcept { _raw_revoke(false); }
		DropTarget() = default;

		DropTarget(const DropTarget&) = delete;
		DropTarget& operator=(const DropTarget&) = delete;
		DropTarget(DropTarget&&) = delete;
		DropTarget& operator=(DropTarget&&) = delete;

		DropTarget& register_drag_drop(HWND hWnd);
		DropTarget& register_drag_drop(const BaseDialog *pDlg)    { return register_drag_drop(pDlg->hwnd()); }
		DropTarget& register_drag_drop(const BaseNativeCtrl &ctl) { return register_drag_drop(ctl.hwnd()); }
		DropTarget& revoke_drag_drop(); // Automatically called during WM_DESTROY of _hWnd.

		DropTarget& on_drag_enter(std::function<bool(const std::vector<std::wstring> &files)> cb); // Returning false from the lambda will prevent the drop.
		DropTarget& on_drop(std::function<void(const std::vector<std::wstring> &files)> cb); // Called only of on_drag_enter() returned true.
	private:
		DropTarget& _raw_revoke(bool canThrow);
		static LRESULT CALLBACK _subclass_proc(HWND hWnd, UINT uMsg, WPARAM wp, LPARAM lp, UINT_PTR idSubclass, DWORD_PTR refData) noexcept;

		wd::ComPtr<Impl> _p{new Impl{}}; // instantiate immediately
		HWND _hWnd = nullptr;
	};

}
