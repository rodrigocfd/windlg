#include <mutex>
#include <thread>
#include "sys-func.hpp"
#include "com.hpp"
#include "file.hpp"
#include <ShObjIdl.h>
using namespace wd;

int wd::run_cmd_sync(StrView cmdLine) {
	SECURITY_ATTRIBUTES sa{
		.nLength = sizeof(SECURITY_ATTRIBUTES),
		.bInheritHandle = TRUE,
	};
	STARTUPINFOW si{
		.cb = sizeof(STARTUPINFOW),
		.dwFlags = STARTF_USESHOWWINDOW,
		.wShowWindow = SW_SHOW,
	};
	PROCESS_INFORMATION pi{};
	DWORD exitCode = 1; // returned by executed program
	std::wstring cmdLine2{cmdLine}; // https://devblogs.microsoft.com/oldnewthing/20090601-00/?p=18083

	if (!CreateProcessW(nullptr, cmdLine2.data(), &sa, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) [[unlikely]] {
		throw WinErr{GetLastError(), L"CreateProcess failed."};
	}

	WaitForSingleObject(pi.hProcess, INFINITE); // block the thread until the process finishes
	GetExitCodeProcess(pi.hProcess, &exitCode);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	return exitCode;
}

void wd::run_thread_pool(size_t numRuns, std::function<void(size_t)> fun, size_t maxThreads) {
	if (!numRuns || !maxThreads) [[unlikely]] {
		return;
	}

	if (maxThreads == std::dynamic_extent) {
		SYSTEM_INFO si{};
		GetSystemInfo(&si);
		maxThreads = si.dwNumberOfProcessors;
	}

	static size_t nextIndex = 0;
	nextIndex = 0;

	for (size_t i = 0; i < std::min(numRuns, maxThreads); ++i) {
		std::thread{[numRuns, fun]() {
			for (;;) {
				size_t ourIndex = 0;
				{
					static std::mutex mutex{};
					std::lock_guard lock{mutex};
					ourIndex = nextIndex++;
					if (ourIndex >= numRuns)
						break;
				}
				fun(ourIndex);
			}
		}}.detach();
	}
}


StopWatch::StopWatch() noexcept
	: _freq{}, _t0{}
{
	QueryPerformanceFrequency(&_freq);
	restart();
}

StopWatch& StopWatch::restart() noexcept {
	QueryPerformanceCounter(&_t0);
	return *this;
}

size_t StopWatch::ellapsed_ms() const noexcept {
	LARGE_INTEGER tNow{};
	QueryPerformanceCounter(&tNow);

	const ULONGLONG ticks = static_cast<ULONGLONG>(tNow.QuadPart - _t0.QuadPart);
	return static_cast<size_t>((ticks * 1000 + _freq.QuadPart / 2) / _freq.QuadPart); // round ms
}

std::wstring StopWatch::ellapsed_fmt() const {
	const size_t totalMs = ellapsed_ms();
	const size_t SEC  = 1000;
	const size_t MIN  = 60 * SEC;
	const size_t HOUR = 60 * MIN;

	const size_t hour = (totalMs - (totalMs % HOUR)) / HOUR;
	size_t min = totalMs - hour * HOUR;
	min = (min - (min % MIN)) / MIN;
	size_t sec = totalMs - hour * HOUR - min * MIN;
	sec = (sec - (sec % SEC)) / SEC;
	const size_t ms = totalMs - hour * HOUR - min * MIN - sec * SEC;

	if      (hour > 0) return wd::str::fmt(L"%u:%02u:%02u.%03u", hour, min, sec, ms);
	if      (min > 0)  return wd::str::fmt(L"%u:%02u.%03u", min, sec, ms);
	else if (sec > 0)  return wd::str::fmt(L"%g s", static_cast<float>(totalMs) / SEC);
	else               return wd::str::fmt(L"%u ms", totalMs);
}


static bool taskdlg_base(const BaseContainer *pOwner, PCWSTR icon, bool isAsk,
	StrView title, StrView top, StrView body, StrView okBtn)
{
	const TASKDIALOG_BUTTON customBtns[] = {
		{.nButtonID = IDOK,     .pszButtonText = okBtn.empty() ? L"&OK" : okBtn.c_str()},
		{.nButtonID = IDCANCEL, .pszButtonText = L"&Cancel"},
	};
	const UINT nBtns = isAsk ? 2 : 1;

	const TASKDIALOGCONFIG tdc{
		.cbSize = sizeof(TASKDIALOGCONFIG),
		.hwndParent = pOwner ? pOwner->hwnd() : nullptr,
		.hInstance = HIWORD(icon) ? nullptr : GetModuleHandleW(nullptr), // TD_*_ICON constants will have HIWORD set
		.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_POSITION_RELATIVE_TO_WINDOW,
		.pszWindowTitle = title.c_str(),
		.pszMainIcon = icon,
		.pszMainInstruction = top.c_str(),
		.pszContent = body.c_str(),
		.cButtons = nBtns,
		.pButtons = customBtns,
	};

	int pnButton = 0;
	if (HRESULT hr = TaskDialogIndirect(&tdc, &pnButton, nullptr, nullptr); FAILED(hr)) [[unlikely]] {
		throw WinErr{hr, L"TaskDialogIndirect failed."};
	}

	return pnButton == IDOK;
}

void wd::sys_dlg::msg_ok(const BaseContainer *pOwner, StrView title, StrView body) {
	taskdlg_base(pOwner, TD_INFORMATION_ICON, false, title, nullptr, body, nullptr);
}

void wd::sys_dlg::msg_warn(const BaseContainer *pOwner, StrView title, StrView body) {
	taskdlg_base(pOwner, TD_WARNING_ICON, false, title, nullptr, body, nullptr);
}

void wd::sys_dlg::msg_err(const BaseContainer *pOwner, StrView title, StrView body) {
	taskdlg_base(pOwner, TD_ERROR_ICON, false, title, nullptr, body, nullptr);
}

void wd::sys_dlg::msg_err(const BaseContainer *pOwner, StrView title, StrView body, const WinErr &e) {
	if (body.empty()) {
		msg_err(pOwner, title, e.msg());
	} else {
		msg_err(pOwner, title, str::fmt(L"%s\n\n%s", body, e.msg()));
	}
}

void wd::sys_dlg::msg_err(const BaseContainer *pOwner, StrView title, StrView body, const std::exception &e) {
	std::wstring body2{};
	if (!body.empty())
		body2 = str::fmt(L"%s\n\n", body);

	body2 += str::to_wide(e.what());
	msg_err(pOwner, title, body2);
}

bool wd::sys_dlg::msg_ask(const BaseContainer *pOwner, StrView title, StrView body, StrView okBtn) {
	return taskdlg_base(pOwner, TD_WARNING_ICON, true, title, nullptr, body, okBtn);
}

void wd::sys_dlg::msg_about(const BaseContainer *pOwner, WORD iconId) {
	VersionInfo ver{};
	const std::wstring cdate = str::to_wide(__DATE__), ctime = str::to_wide(__TIME__);
	const std::wstring msg = str::fmt(
		L"%s\n"
		L"Written in C++20 and WinDlg\n"
		L"Version %u.%u.%u\n"
		L"Built %s %s",
		ver.legalCopyright,
		ver.version[0], ver.version[1], ver.version[2],
		cdate, ctime);

	taskdlg_base(pOwner, iconId ? MAKEINTRESOURCEW(iconId) : TD_INFORMATION_ICON,
		false, L"About", ver.fileDescription, msg, nullptr);
}


std::wstring wd::sys_dlg::file_open(const BaseContainer *pOwner,
	std::initializer_list<COMDLG_FILTERSPEC> filters)
{
	ComPtr<IFileOpenDialog> fod{};
	fod.co_create_instance(CLSID_FileOpenDialog);

	FILEOPENDIALOGOPTIONS defOpts = 0;
	fod->GetOptions(&defOpts);
	fod->SetOptions(defOpts | FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST);

	fod->SetFileTypes(static_cast<UINT>(filters.size()), filters.begin());
	fod->SetFileTypeIndex(1); // one-based

	if (fod->Show(pOwner->hwnd()) == HRESULT_FROM_WIN32(ERROR_CANCELLED))
		return {}; // user cancelled

	ComPtr<IShellItem> item{};
	if (HRESULT hr = fod->GetResult(item.pptr()); FAILED(hr)) [[unlikely]] {
		throw WinErr{hr, L"IFileOpenDialog::GetResult failed."};
	}

	CoString fn{};
	item->GetDisplayName(SIGDN_FILESYSPATH, fn.pptr());
	return {fn.c_str()};
}

std::vector<std::wstring> wd::sys_dlg::file_open_multi(const BaseContainer *pOwner,
	std::initializer_list<COMDLG_FILTERSPEC> filters)
{
	ComPtr<IFileOpenDialog> fod{};
	fod.co_create_instance(CLSID_FileOpenDialog);

	FILEOPENDIALOGOPTIONS defOpts = 0;
	fod->GetOptions(&defOpts);
	fod->SetOptions(defOpts | FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_ALLOWMULTISELECT);

	fod->SetFileTypes(static_cast<UINT>(filters.size()), filters.begin());
	fod->SetFileTypeIndex(1); // one-based

	if (fod->Show(pOwner->hwnd()) == HRESULT_FROM_WIN32(ERROR_CANCELLED))
		return {}; // user cancelled

	ComPtr<IShellItemArray> arr{};
	if (HRESULT hr = fod->GetResults(arr.pptr()); FAILED(hr)) [[unlikely]] {
		throw WinErr{hr, L"IFileOpenDialog::GetResults failed."};
	}

	DWORD count = 0;
	arr->GetCount(&count);

	std::vector<std::wstring> fns{}; // to be returned
	fns.reserve(count);

	for (DWORD i = 0; i < count; ++i) {
		ComPtr<IShellItem> item{};
		arr->GetItemAt(i, item.pptr());

		CoString fn{};
		item->GetDisplayName(SIGDN_FILESYSPATH, fn.pptr());
		fns.emplace_back(fn.c_str());
	}

	return fns;
}

std::wstring wd::sys_dlg::file_save(const BaseContainer *pOwner,
	std::initializer_list<COMDLG_FILTERSPEC> filters)
{
	ComPtr<IFileSaveDialog> fsd{};
	fsd.co_create_instance(CLSID_FileSaveDialog);

	FILEOPENDIALOGOPTIONS defOpts = 0;
	fsd->GetOptions(&defOpts);
	fsd->SetOptions(defOpts | FOS_FORCEFILESYSTEM | FOS_STRICTFILETYPES);

	fsd->SetFileTypes(static_cast<UINT>(filters.size()), filters.begin());
	fsd->SetFileTypeIndex(1); // one-based

	const COMDLG_FILTERSPEC &firstFilterSpec = *filters.begin();
	std::vector<std::wstring> parts = StrView{firstFilterSpec.pszSpec}.split_n(L";", 2);
	while (parts[0].starts_with(L'*') || parts[0].starts_with(L'.'))
		parts[0].erase(0, 1); // 1st extension will be used
	if (!parts[0].empty())
		fsd->SetDefaultExtension(parts[0].c_str()); // so that chosen extension is automatically appended to file name

	if (fsd->Show(pOwner->hwnd()) == HRESULT_FROM_WIN32(ERROR_CANCELLED))
		return {}; // user cancelled

	ComPtr<IShellItem> item{};
	if (HRESULT hr = fsd->GetResult(item.pptr()); FAILED(hr)) [[unlikely]] {
		throw WinErr{hr, L"IFileSaveDialog::GetResult failed."};
	}

	CoString fn{};
	item->GetDisplayName(SIGDN_FILESYSPATH, fn.pptr());
	return {fn.c_str()};
}

std::wstring wd::sys_dlg::folder_open(const BaseContainer *pOwner, StrView defFolder) {
	ComPtr<IFileOpenDialog> fod{};
	fod.co_create_instance(CLSID_FileOpenDialog);

	FILEOPENDIALOGOPTIONS defOpts = 0;
	fod->GetOptions(&defOpts);
	fod->SetOptions(defOpts | FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_PICKFOLDERS);

	ComPtr<IShellItem> ishDefFolder{};
	if (!defFolder.empty() && file::is_dir(defFolder)) {
		SHCreateItemFromParsingName(defFolder.c_str(), nullptr, IID_IShellItem, ishDefFolder.pptr<void>());
		fod->SetFolder(ishDefFolder.ptr());
	}

	if (fod->Show(pOwner->hwnd()) == HRESULT_FROM_WIN32(ERROR_CANCELLED))
		return {}; // user cancelled

	ComPtr<IShellItem> item{};
	if (HRESULT hr = fod->GetResult(item.pptr()); FAILED(hr)) [[unlikely]] {
		throw WinErr{hr, L"IFileOpenDialog::GetResult failed."};
	}

	CoString fn{};
	item->GetDisplayName(SIGDN_FILESYSPATH, fn.pptr());
	return {fn.c_str()};
}


FILETIME& wd::time::add_msec(FILETIME &ft, size_t numMsecs) noexcept {
	reinterpret_cast<ULARGE_INTEGER*>(&ft)->QuadPart += numMsecs * 10'000;
	return ft;
}

SYSTEMTIME& wd::time::add_msec(SYSTEMTIME &st, size_t numMsecs) noexcept {
	FILETIME ft = to_filetime(st);
	add_msec(ft, numMsecs);
	st = to_systemtime(ft);
	return st;
}

size_t wd::time::diff_ms(const FILETIME &a, const FILETIME &b) noexcept {
	ULONGLONG uA = reinterpret_cast<const ULARGE_INTEGER*>(&a)->QuadPart;
	ULONGLONG uB = reinterpret_cast<const ULARGE_INTEGER*>(&b)->QuadPart;
	return static_cast<size_t>( (uA > uB ? (uA - uB) : (uB - uA)) / 10'000 );
}

size_t wd::time::diff_ms(const SYSTEMTIME &a, const SYSTEMTIME &b) noexcept {
	return diff_ms(to_filetime(a), to_filetime(b));
}

FILETIME wd::time::now() noexcept {
	FILETIME ftUtc{}, ftLocal{};
	GetSystemTimePreciseAsFileTime(&ftUtc);
	FileTimeToLocalFileTime(&ftUtc, &ftLocal);
	return ftLocal;
}

FILETIME wd::time::to_filetime(const SYSTEMTIME &st) noexcept {
	FILETIME ft{};
	SystemTimeToFileTime(&st, &ft);
	return ft;
}

SYSTEMTIME wd::time::to_systemtime(const FILETIME &ft) noexcept {
	SYSTEMTIME st{};
	FileTimeToSystemTime(&ft, &st);
	return st;
}
