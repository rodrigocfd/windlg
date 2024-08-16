#include <algorithm>
#include <memory>
#include <system_error>
#include <thread>
#include <Windows.h>
#include <CommCtrl.h>
#include <ShObjIdl.h>
#include "Dialog.h"
#include "Com.h"
using namespace lib;

struct ThreadPack final {
	std::function<void()> callback;
	std::exception_ptr pCurExcep;
};
static constexpr UINT WM_THREAD = WM_APP + 0x3fff;


void Dialog::Facilities::enable(std::initializer_list<WORD> ctrlIds, bool doEnable) const
{
	for (auto&& ctrlId : ctrlIds)
		EnableWindow(GetDlgItem(_pDlg->hWnd(), ctrlId), doEnable);
}

const Dialog::Facilities& Dialog::Facilities::layout(Horz horz, Vert vert, std::initializer_list<WORD> ctrlIds) const
{
	_pDlg->_layout.add(horz, vert, ctrlIds);
	return *this;
}

const Dialog::Facilities& Dialog::Facilities::registerDragDrop() const
{
	if (_pDlg->_usingDropTarget) [[unlikely]] {
		throw std::logic_error("RegisterDragDrop called twice");
	}

	DWORD exStyle = static_cast<DWORD>(GetWindowLongPtrW(_pDlg->hWnd(), GWL_EXSTYLE));
	if (exStyle & WS_EX_ACCEPTFILES) [[unlikely]] {
		throw std::invalid_argument("Do not use WS_EX_ACCEPTFILES");
	}

	if (HRESULT hr = RegisterDragDrop(_pDlg->hWnd(), &_pDlg->_dropTarget); hr == E_OUTOFMEMORY) [[unlikely]] {
		throw std::runtime_error("RegisterDragDrop failed; did you instantiate lib::ComOle?");
	} else if (FAILED(hr)) [[unlikely]] {
		throw std::system_error(GetLastError(), std::system_category(), "RegisterDragDrop failed");
	}
	_pDlg->_usingDropTarget = true;
	return *this;
}

void Dialog::Facilities::runDetachedThread(std::function<void()> callback) const
{
	std::thread([callback = std::move(callback), this]() {
		try {
			callback();
		} catch (...) {
			auto pPack = std::make_unique<ThreadPack>([]{ }, std::current_exception());
			SendMessageW(_pDlg->hWnd(), WM_THREAD, WM_THREAD, reinterpret_cast<LPARAM>(pPack.release()));
		}
	}).detach();
}

void Dialog::Facilities::runUiThread(std::function<void()> callback) const
{
	auto pPack = std::make_unique<ThreadPack>(std::move(callback), nullptr);
	SendMessageW(_pDlg->hWnd(), WM_THREAD, WM_THREAD, reinterpret_cast<LPARAM>(pPack.release()));
}

static std::vector<COMDLG_FILTERSPEC> _makeFilters(
	std::initializer_list<std::pair<std::wstring_view, std::wstring_view>> namesExts)
{
	std::vector<COMDLG_FILTERSPEC> rawFilters; // {L"Word Document (*.doc)", L"*.doc"}
	rawFilters.reserve(namesExts.size());
	for (const auto& nameExt : namesExts)
		rawFilters.emplace_back(nameExt.first.data(), nameExt.second.data());
	return rawFilters;
}

static std::wstring _shellItemPath(const ComPtr<IShellItem>& shi)
{
	LPWSTR ptrPath = nullptr;
	if (HRESULT hr = shi->GetDisplayName(SIGDN_FILESYSPATH, &ptrPath); FAILED(hr)) [[unlikely]] {
		throw std::system_error(hr, std::system_category(), "IShellItem::GetDisplayName failed");
	}
	std::wstring strPath{(ptrPath == nullptr ? L"" : ptrPath)};
	CoTaskMemFree(ptrPath);
	return strPath;
}

std::optional<std::vector<std::wstring>> Dialog::Facilities::showOpenFiles(
	std::initializer_list<std::pair<std::wstring_view, std::wstring_view>> namesExts) const
{
	std::vector<COMDLG_FILTERSPEC> filters = _makeFilters(namesExts);

	ComPtr<IFileOpenDialog> fsd{CLSID_FileOpenDialog};
	fsd->SetOptions(FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_ALLOWMULTISELECT);
	fsd->SetFileTypes(static_cast<int>(filters.size()), filters.data());
	fsd->SetFileTypeIndex(1);
	if (HRESULT hr = fsd->Show(_pDlg->hWnd()); hr == S_OK) {
		ComPtr<IShellItemArray> shiArr;
		fsd->GetResults(shiArr.pptr());

		DWORD count = 0;
		shiArr->GetCount(&count);

		std::vector<std::wstring> strPaths;
		strPaths.reserve(count);
		for (size_t i = 0; i < count; ++i) {
			ComPtr<IShellItem> shi;
			shiArr->GetItemAt(static_cast<DWORD>(i), shi.pptr());
			strPaths.emplace_back(_shellItemPath(shi));
		}

		std::sort(strPaths.begin(), strPaths.end(), [](const auto& a, const auto& b) -> bool {
			return lstrcmpiW(a.c_str(), b.c_str()) < 1;
		});
		return strPaths;
	} else if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED)) {
		return std::nullopt;
	} else [[unlikely]] {
		throw std::system_error(hr, std::system_category(), "IModalWindow::Show failed");
	}
}

std::optional<std::wstring> Dialog::Facilities::_showOpenSave(bool isOpen, bool isFolder,
	std::initializer_list<std::pair<std::wstring_view, std::wstring_view>> namesExts) const
{
	std::vector<COMDLG_FILTERSPEC> filters = _makeFilters(namesExts);

	ComPtr<IFileDialog> fd{(isOpen ? CLSID_FileOpenDialog : CLSID_FileSaveDialog)};
	fd->SetOptions(FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | (isFolder ? FOS_PICKFOLDERS : 0));
	if (!isFolder) {
		fd->SetFileTypes(static_cast<int>(filters.size()), filters.data());
		fd->SetFileTypeIndex(1);
	}
	if (HRESULT hr = fd->Show(_pDlg->hWnd()); hr == S_OK) {
		ComPtr<IShellItem> shi;
		fd->GetResult(shi.pptr());
		return _shellItemPath(shi);
	} else if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED)) {
		return std::nullopt;
	} else [[unlikely]] {
		throw std::system_error(hr, std::system_category(), "IModalWindow::Show failed");
	}
}

int Dialog::Facilities::msgBox(std::wstring_view title, std::optional<std::wstring_view> mainInstruction,
	std::wstring_view body, int tdcbfButtons, LPWSTR tdIcon) const
{
	TASKDIALOGCONFIG tdc = {
		.cbSize = sizeof(TASKDIALOGCONFIG),
		.hwndParent = _pDlg->hWnd(),
		.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_POSITION_RELATIVE_TO_WINDOW,
		.dwCommonButtons = tdcbfButtons,
		.pszWindowTitle = title.data(),
		.pszMainIcon = tdIcon,
		.pszMainInstruction = mainInstruction.has_value() ? mainInstruction.value().data() : nullptr,
		.pszContent = body.data(),
	};

	int pnButton = 0;
	if (HRESULT hr = TaskDialogIndirect(&tdc, &pnButton, nullptr, nullptr); FAILED(hr)) [[unlikely]] {
		throw std::system_error(hr, std::system_category(), "TaskDialogIndirect failed");
	}
	return pnButton;
}


void Dialog::Layout::add(Horz horz, Vert vert, std::initializer_list<WORD> ctrlIds)
{
	if (horz == Horz::None && vert == Vert::None) [[unlikely]] { // if nothing to do, don't even bother adding it
		return;
	}

	if (_ctrls.empty()) { // first controls being added
		RECT rc{};
		GetClientRect(_pDlg->hWnd(), &rc);
		_szParentOrig = {.cx = rc.right, .cy = rc.bottom};
	}

	for (auto&& ctrlId : ctrlIds) {
		HWND hCtrl = GetDlgItem(_pDlg->hWnd(), ctrlId);
		RECT rc{};
		GetWindowRect(hCtrl, &rc);
		ScreenToClient(_pDlg->hWnd(), reinterpret_cast<POINT*>(&rc));
		ScreenToClient(_pDlg->hWnd(), reinterpret_cast<POINT*>(&rc.right));
		_ctrls.emplace_back(hCtrl, horz, vert, rc);
	}
}

void Dialog::Layout::processMsgs(UINT uMsg, WPARAM wp, LPARAM lp) const
{
	if (_ctrls.empty() || uMsg != WM_SIZE || wp == SIZE_MINIMIZED) return;

	UINT cxParent = LOWORD(lp);
	UINT cyParent = HIWORD(lp);

	HDWP hdwp = BeginDeferWindowPos(static_cast<int>(_ctrls.size()));
	for (auto& ctrl : _ctrls) {
		UINT swp = SWP_NOZORDER;
		if (ctrl.horz == Horz::Repos && ctrl.vert == Vert::Repos) { // reposition both horz & vert
			swp |= SWP_NOSIZE;
		} else if (ctrl.horz == Horz::Resize && ctrl.vert == Vert::Resize) { // resize both horz & vert
			swp |= SWP_NOMOVE;
		}

		DeferWindowPos(hdwp, ctrl.hCtrl, nullptr,
			ctrl.horz == Horz::Repos ? cxParent - _szParentOrig.cx + ctrl.rcOrig.left
				: ctrl.rcOrig.left,
			ctrl.vert == Vert::Repos ? cyParent - _szParentOrig.cy + ctrl.rcOrig.top
				: ctrl.rcOrig.top,
			ctrl.horz == Horz::Resize ? cxParent - _szParentOrig.cx + ctrl.rcOrig.right - ctrl.rcOrig.left
				: ctrl.rcOrig.right - ctrl.rcOrig.left,
			ctrl.vert == Vert::Resize ? cyParent - _szParentOrig.cy + ctrl.rcOrig.bottom - ctrl.rcOrig.top
				: ctrl.rcOrig.bottom - ctrl.rcOrig.top,
			swp);
	}
	EndDeferWindowPos(hdwp);
}


HRESULT STDMETHODCALLTYPE Dialog::DropTarget::QueryInterface(REFIID riid, void** ppvObject)
{
	if (riid == IID_IDropTarget || riid == IID_IUnknown) {
		AddRef();
		*ppvObject = this;
		return S_OK;
	} else {
		*ppvObject = nullptr;
		return E_NOINTERFACE;
	}
}

ULONG STDMETHODCALLTYPE Dialog::DropTarget::AddRef()
{
	return InterlockedIncrement(&_refCount);
}

ULONG STDMETHODCALLTYPE Dialog::DropTarget::Release()
{
	return InterlockedDecrement(&_refCount);
}

HRESULT STDMETHODCALLTYPE Dialog::DropTarget::DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
	*pdwEffect &= DROPEFFECT_COPY;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE Dialog::DropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
	*pdwEffect &= DROPEFFECT_COPY;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE Dialog::DropTarget::Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
	FORMATETC fetc = {
		.cfFormat = CF_HDROP,
		.ptd = nullptr,
		.dwAspect = DVASPECT_CONTENT,
		.lindex = -1,
		.tymed = TYMED_HGLOBAL,
	};
	STGMEDIUM medium{};

	if (HRESULT hr = pDataObj->GetData(&fetc, &medium); FAILED(hr)) {
		*pdwEffect = DROPEFFECT_NONE;
		return hr;
	}

	HGLOBAL hFiles = medium.hGlobal;
	auto hDrop = reinterpret_cast<HDROP>(GlobalLock(hFiles));	
	auto files = _getDropped(hDrop);
	GlobalUnlock(hFiles);
	ReleaseStgMedium(&medium);

	_pDlg->onDropTarget(files);
	*pdwEffect &= DROPEFFECT_COPY;
	return S_OK;
}

std::vector<std::wstring> Dialog::DropTarget::_getDropped(HDROP hDrop) const
{
	UINT count = DragQueryFileW(hDrop, 0xffff'ffff, nullptr, 0);
	std::vector<std::wstring> paths;
	paths.reserve(count);

	for (UINT i = 0; i < count; ++i) {
		WCHAR buf[MAX_PATH + 1] = {L'\0'};
		DragQueryFileW(hDrop, i, buf, MAX_PATH + 1);
		paths.emplace_back(buf);
	}

	//DragFinish(hDrop); // will crash ReleaseStgMedium()
	return paths;
}


INT_PTR CALLBACK Dialog::_DlgProc(HWND hDlg, UINT uMsg, WPARAM wp, LPARAM lp)
{
	Dialog *pSelf = nullptr;
	
	if (uMsg == WM_INITDIALOG) {
		pSelf = reinterpret_cast<Dialog*>(lp);
		SetWindowLongPtrW(hDlg, DWLP_USER, reinterpret_cast<LONG_PTR>(pSelf));
		*pSelf->_hWndPtr() = hDlg;
	} else [[likely]] {
		pSelf = reinterpret_cast<Dialog*>(GetWindowLongPtrW(hDlg, DWLP_USER));
	}

	if (!pSelf) [[unlikely]] {
		// No pointer stored, nothing is done.
		// Prevents processing before WM_INITDIALOG and after WM_NCDESTROY.
		return FALSE;
	}

	pSelf->_layout.processMsgs(uMsg, wp, lp);

	if (uMsg == WM_THREAD && wp == WM_THREAD) // incoming from another thread through SendMessage()
		pSelf->_runFromOtherThread(lp);

	INT_PTR userRet = pSelf->dlgProc(uMsg, wp, lp);

	if (uMsg == WM_NCDESTROY) {
		if (pSelf->_usingDropTarget) RevokeDragDrop(hDlg);
		SetWindowLongPtrW(hDlg, DWLP_USER, 0);
		*pSelf->_hWndPtr() = nullptr;
		userRet = TRUE;
	}
	return userRet;
}

void Dialog::_Lippincott()
{
	LPCSTR caption = "Uncaught unknown exception";
	LPCSTR msg = "An unknown exception, not derived from std::exception, was thrown.";

	// https://stackoverflow.com/a/48036486/6923555
	try { throw; }
	catch (const std::invalid_argument& e) { msg = e.what(); caption = "Uncaught invalid argument"; }
	catch (const std::out_of_range& e)     { msg = e.what(); caption = "Uncaught out of range"; }
	catch (const std::logic_error& e)      { msg = e.what(); caption = "Uncaught logic error"; }
	catch (const std::system_error& e)     { msg = e.what(); caption = "Uncaught system error"; }
	catch (const std::runtime_error& e)    { msg = e.what(); caption = "Uncaught runtime error"; }
	catch (const std::exception& e)        { msg = e.what(); caption = "Uncaught generic error"; }

	MessageBoxA(nullptr, msg, caption, MB_ICONERROR);
}

void Dialog::_runFromOtherThread(LPARAM lp) const
{
	std::unique_ptr<ThreadPack> pPack{reinterpret_cast<ThreadPack*>(lp)};
	if (pPack->pCurExcep) { // catching an exception from runDetachedThread()
		try {
			std::rethrow_exception(pPack->pCurExcep);
		} catch (...) {
			_Lippincott();
			PostQuitMessage(1);
		}
	} else { // from runUiThread()
		try {
			pPack->callback();
		} catch (...) {
			_Lippincott();
			PostQuitMessage(1);
		}
	}
}
