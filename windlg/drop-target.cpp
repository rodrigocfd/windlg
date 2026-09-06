#include "drop-target.hpp"
#include "win-error.hpp"
#include <shellapi.h>
using namespace wd;

OleInit::OleInit() {
	if (HRESULT hr = OleInitialize(nullptr); FAILED(hr)) [[unlikely]] {
		throw WinErr{hr, L"OleInitialize failed."};
	}
}


HRESULT STDMETHODCALLTYPE DropTarget::Impl::QueryInterface(REFIID riid, void **ppvObject) {
	if (riid == IID_IDropTarget || riid == IID_IUnknown) {
		AddRef();
		*ppvObject = this;
		return S_OK;
	}
	*ppvObject = nullptr;
	return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE DropTarget::Impl::AddRef() {
	return InterlockedIncrement(&refCount);
}

ULONG STDMETHODCALLTYPE DropTarget::Impl::Release() {
	const ULONG c = InterlockedDecrement(&refCount);
	if (!refCount)
		delete this;
	return c;
}

HRESULT STDMETHODCALLTYPE DropTarget::Impl::DragEnter(IDataObject *pDataObj,
	[[maybe_unused]] DWORD grfKeyState, [[maybe_unused]] POINTL pt, DWORD *pdwEffect)
{
	if (on_drag_enter) { // user specified a callback
		proceedDrop = on_drag_enter(get_files(pDataObj)); // will Drop happen?
		*pdwEffect = proceedDrop ? DROPEFFECT_COPY : DROPEFFECT_NONE;
	} else {
		proceedDrop = true; // user didn't specify a callback, by default Drop will happen
		*pdwEffect = DROPEFFECT_COPY;
	}
	return S_OK;
}

HRESULT STDMETHODCALLTYPE DropTarget::Impl::DragOver(
	[[maybe_unused]] DWORD grfKeyState, [[maybe_unused]] POINTL pt, DWORD *pdwEffect)
{
	*pdwEffect = proceedDrop ? DROPEFFECT_COPY : DROPEFFECT_NONE; // preserve from DragEnter
	return S_OK;
}

HRESULT STDMETHODCALLTYPE DropTarget::Impl::Drop(IDataObject *pDataObj,
	[[maybe_unused]] DWORD grfKeyState, [[maybe_unused]] POINTL pt, DWORD *pdwEffect)
{
	if (proceedDrop && on_drop)
		on_drop(get_files(pDataObj));

	proceedDrop = true; // restore default
	*pdwEffect = DROPEFFECT_COPY;
	return S_OK;
}

std::vector<std::wstring> DropTarget::Impl::get_files(IDataObject *pDataObj) const {
	FORMATETC fetc{
		.cfFormat = CF_HDROP,
		.ptd = nullptr,
		.dwAspect = DVASPECT_CONTENT,
		.lindex = -1,
		.tymed = TYMED_HGLOBAL,
	};
	STGMEDIUM medium{};

	if (HRESULT hr = pDataObj->GetData(&fetc, &medium); FAILED(hr)) [[unlikely]] {
		return {};
	}

	const HGLOBAL hFiles = medium.hGlobal;
	const HDROP hDrop = reinterpret_cast<HDROP>(GlobalLock(hFiles));

#ifdef _MSC_VER
#pragma warning(suppress: 6387)
#endif
	const UINT count = DragQueryFileW(hDrop, 0xffff'ffff, nullptr, 0);
	std::vector<std::wstring> paths{};
	paths.reserve(count);
	WCHAR buf[MAX_PATH] = {0};
	for (UINT i = 0; i < count; ++i) {
		DragQueryFileW(hDrop, i, buf, ARRAYSIZE(buf));
		paths.emplace_back(buf);
	}

	//DragFinish(hDrop); // will crash ReleaseStgMedium()
	GlobalUnlock(hFiles);
	ReleaseStgMedium(&medium);
	return paths;
}


DropTarget& DropTarget::register_drag_drop(HWND hWnd) {
	if (HRESULT hr = RegisterDragDrop(hWnd, _p.ptr()); FAILED(hr)) [[unlikely]] {
		throw WinErr{hr, L"RegisterDragDrop failed."};
	}
	_hWnd = hWnd;

	static UINT idSubclass = 1;
	if (BOOL ok = SetWindowSubclass(hWnd, _subclass_proc, idSubclass++, reinterpret_cast<DWORD_PTR>(this)); !ok) [[unlikely]] {
		throw WinErr{ERROR_UNIDENTIFIED_ERROR, L"SetWindowSubclass failed."};
	}
	return *this;
}

DropTarget& DropTarget::revoke_drag_drop() {
	return _raw_revoke(true);
}

DropTarget& DropTarget::on_drag_enter(std::function<bool(const std::vector<std::wstring> &files)> cb) {
	_p->on_drag_enter = std::move(cb);
	return *this;
}

DropTarget& DropTarget::on_drop(std::function<void(const std::vector<std::wstring> &files)> cb) {
	_p->on_drop = std::move(cb);
	return *this;
}

DropTarget& DropTarget::_raw_revoke(bool canThrow) {
	if (_hWnd) {
		if (HRESULT hr = RevokeDragDrop(_hWnd); FAILED(hr) && canThrow) [[unlikely]] {
			throw WinErr{hr, L"RevokeDragDrop failed."};
		}
		_hWnd = nullptr;
	}
	return *this;
}

LRESULT CALLBACK DropTarget::_subclass_proc(HWND hWnd, UINT uMsg,
	WPARAM wp, LPARAM lp, UINT_PTR idSubclass, DWORD_PTR refData) noexcept
{
	if (uMsg == WM_DESTROY) {
		DropTarget *pSelf = reinterpret_cast<DropTarget*>(refData);
		pSelf->_raw_revoke(false);
	} else if (uMsg == WM_NCDESTROY) {
		RemoveWindowSubclass(hWnd, _subclass_proc, idSubclass);
	}
	return DefSubclassProc(hWnd, uMsg, wp, lp);
}
