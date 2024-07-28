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

std::optional<std::vector<std::wstring>> Dialog::SysDlgs::openFiles(
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

std::optional<std::wstring> Dialog::SysDlgs::_openSave(bool isOpen, bool isFolder,
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

int Dialog::SysDlgs::msgBox(std::wstring_view title, std::wstring_view mainInstruction,
	std::wstring_view body, int tdcbfButtons, LPWSTR tdIcon) const
{
	TASKDIALOGCONFIG tdc = {
		.cbSize = sizeof(TASKDIALOGCONFIG),
		.hwndParent = _pDlg->hWnd(),
		.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_POSITION_RELATIVE_TO_WINDOW,
		.dwCommonButtons = tdcbfButtons,
		.pszWindowTitle = title.data(),
		.pszMainIcon = tdIcon,
		.pszMainInstruction = mainInstruction.empty() ? nullptr : mainInstruction.data(),
		.pszContent = body.data(),
	};

	int pnButton = 0;
	if (HRESULT hr = TaskDialogIndirect(&tdc, &pnButton, nullptr, nullptr); FAILED(hr)) [[unlikely]] {
		throw std::system_error(hr, std::system_category(), "TaskDialogIndirect failed");
	}
	return pnButton;
}


struct ThreadPack final {
	std::function<void()> callback;
	std::exception_ptr pCurExcep;
};
static constexpr UINT WM_THREAD = WM_APP + 0x3fff;

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

	if (uMsg == WM_THREAD && wp == WM_THREAD) // incoming from another thread through SendMessage()
		pSelf->_runFromOtherThread(lp);

	INT_PTR userRet = pSelf->dlgProc(uMsg, wp, lp);

	if (uMsg == WM_NCDESTROY) {
		SetWindowLongPtrW(hDlg, DWLP_USER, 0);
		*pSelf->_hWndPtr() = nullptr;
		userRet = TRUE;
	}
	return userRet;
}

std::vector<std::wstring> Dialog::droppedFiles(HDROP hDrop) const
{
	UINT count = DragQueryFileW(hDrop, 0xffff'ffff, nullptr, 0);
	std::vector<std::wstring> paths;
	paths.reserve(count);

	for (UINT i = 0; i < count; ++i) {
		WCHAR buf[MAX_PATH + 1] = {L'\0'};
		DragQueryFileW(hDrop, i, buf, MAX_PATH + 1);
		paths.emplace_back(buf);
	}

	DragFinish(hDrop);
	return paths;
}

void Dialog::enable(std::initializer_list<WORD> ctrlIds, bool doEnable) const
{
	for (auto&& ctrlId : ctrlIds)
		EnableWindow(GetDlgItem(hWnd(), ctrlId), doEnable);
}

void Dialog::runDetachedThread(std::function<void()> callback) const
{
	std::thread([callback = std::move(callback), this]() {
		try {
			callback();
		} catch (...) {
			auto pPack = std::make_unique<ThreadPack>([]{ }, std::current_exception());
			SendMessageW(hWnd(), WM_THREAD, WM_THREAD, reinterpret_cast<LPARAM>(pPack.release()));
		}
	}).detach();
}

void Dialog::runUiThread(std::function<void()> callback) const
{
	auto pPack = std::make_unique<ThreadPack>(std::move(callback), nullptr);
	SendMessageW(hWnd(), WM_THREAD, WM_THREAD, reinterpret_cast<LPARAM>(pPack.release()));
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
