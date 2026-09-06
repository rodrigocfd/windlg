#include <algorithm>
#include "file.hpp"
#include "win-error.hpp"
using namespace wd;

#ifdef _MSC_VER
#pragma comment(lib, "version.lib")
#endif

File& File::operator=(File &&other) noexcept {
	close();
	std::swap(_hFile, other._hFile);
	return *this;
}

File& File::close() noexcept {
	if (_hFile) {
		CloseHandle(_hFile);
		_hFile = nullptr;
	}
	return *this;
}

HANDLE File::leak() noexcept {
	HANDLE h = _hFile;
	_hFile = nullptr;
	return h;
}

File& File::open_read_only(StrView filePath) {
	close();
	_hFile = CreateFileW(filePath.c_str(), GENERIC_READ,
		FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (!_hFile || _hFile == INVALID_HANDLE_VALUE) [[unlikely]] {
		throw WinErr{GetLastError(), L"CreateFile failed."};
	}
	return *this;
}

File& File::open_rw(StrView filePath) {
	close();
	_hFile = CreateFileW(filePath.c_str(), GENERIC_READ | GENERIC_WRITE,
		0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (!_hFile || _hFile == INVALID_HANDLE_VALUE) [[unlikely]] {
		throw WinErr{GetLastError(), L"CreateFile failed."};
	}
	return *this;
}

std::vector<BYTE> File::read(size_t numBytes) const {
	DWORD read = 0;
	std::vector<BYTE> buf(std::min(numBytes, size()), 0x00);

	if (!ReadFile(_hFile, buf.data(), static_cast<DWORD>(buf.size()), &read, nullptr)) [[unlikely]] {
		throw WinErr{GetLastError(), L"ReadFile failed."};
	}

	return buf;
}

size_t File::size() const {
	LARGE_INTEGER sz{};
	if (!GetFileSizeEx(_hFile, &sz)) [[unlikely]] {
		throw WinErr{GetLastError(), L"GetFileSizeEx failed."};
	}
	return static_cast<size_t>(sz.QuadPart);
}

FILETIME File::time_created() const {
	FILETIME ftUtc{}, ftLocal{};
	if (!GetFileTime(_hFile, &ftUtc, nullptr, nullptr)) [[unlikely]] {
		throw WinErr{GetLastError(), L"GetFileTime failed."};
	}
	FileTimeToLocalFileTime(&ftUtc, &ftLocal);
	return ftLocal;
}

FILETIME File::time_last_write() const {
	FILETIME ftUtc{}, ftLocal{};
	if (!GetFileTime(_hFile, nullptr, nullptr, &ftUtc)) [[unlikely]] {
		throw WinErr{GetLastError(), L"GetFileTime failed."};
	}
	FileTimeToLocalFileTime(&ftUtc, &ftLocal);
	return ftLocal;
}

File& File::truncate() {
	LARGE_INTEGER off{}; // zero
	if (!SetFilePointerEx(_hFile, off, nullptr, FILE_BEGIN)) [[unlikely]] {
		throw WinErr{GetLastError(), L"GetFileTime failed."};
	}
	return *this;
}

File& File::write(const BYTE *pSrc, size_t numBytes) {
	DWORD written = 0;
	if (!WriteFile(_hFile, pSrc, static_cast<DWORD>(numBytes), &written, nullptr)) [[unlikely]] {
		throw WinErr{GetLastError(), L"WriteFile failed."};
	}
	return *this;
}


std::wstring wd::file::dir_from(StrView filePath) {
	if (filePath.empty()) [[unlikely]] {
		return {};
	}

	const size_t slashIdx = filePath.find_last_of(L"\\");
	if (slashIdx == StrView::npos) [[unlikely]] {
		return {}; // the whole string is the filename
	}

	return filePath.substr(0, slashIdx); // won't include trailing backslash
}

std::wstring wd::file::exe_dir() {
	WCHAR buf[MAX_PATH] = {0};
	GetModuleFileNameW(nullptr, buf, ARRAYSIZE(buf));
	std::wstring theDir = dir_from(buf);
#ifdef _DEBUG
	theDir = dir_from(theDir); // in debug mode, go up another dir level
#endif
	return theDir;
}

bool wd::file::exists(StrView filePath) noexcept {
	return GetFileAttributesW(filePath.c_str()) != INVALID_FILE_ATTRIBUTES;
}

std::wstring wd::file::filename_from(StrView filePath) {
	if (filePath.empty()) [[unlikely]] {
		return {};
	}

	const size_t slashIdx = filePath.find_last_of(L"\\");
	if (slashIdx == StrView::npos) [[unlikely]] {
		return std::wstring{filePath}; // the whole string is the filename
	}

	return filePath.substr(slashIdx + 1);
}

bool wd::file::is_dir(StrView filePath) noexcept {
	const DWORD attrs = GetFileAttributesW(filePath.c_str());
	return attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY);
}

std::vector<BYTE> wd::file::read(StrView filePath) {
	File f{};
	f.open_read_only(filePath);
	std::vector<BYTE> blob = f.read();
	return blob;
}

std::wstring wd::file::read_ini(StrView iniPath, StrView section, StrView key) {
	DWORD sz = 20;
	std::wstring buf{}; // to be returned

	for (;;) {
		buf.resize(sz);
		DWORD nChars = GetPrivateProfileStringW(section.c_str(), key.c_str(),
			L"", buf.data(), sz, iniPath.c_str());

		DWORD err = GetLastError();
		if (!nChars && err != ERROR_SUCCESS) [[unlikely]] {
			throw WinErr{err, L"GetPrivateProfileString failed."};
		}

		if (nChars < sz - 1)
			return buf;

		sz *= 2; // grow buffer and keep trying
	}
}

std::wstring wd::file::remove_ext(StrView filePath) {
	std::wstring path2{filePath};
	if (size_t dotPos = path2.find_last_of(L'.'); dotPos != std::wstring::npos) [[likely]] {
		path2.resize(dotPos);
	}
	return path2;
}

size_t wd::file::size(StrView filePath) {
	File f{};
	f.open_read_only(filePath);
	return f.size();
}

FILETIME wd::file::time_created(StrView filePath) {
	File f{};
	f.open_read_only(filePath);
	return f.time_created();
}

FILETIME wd::file::time_last_write(StrView filePath) {
	File f{};
	f.open_read_only(filePath);
	return f.time_last_write();
}

void wd::file::write(StrView filePath, const BYTE *pSrc, size_t numBytes) {
	File f{};
	f.open_rw(filePath);
	f.truncate();
	f.write(pSrc, numBytes);
}

void wd::file::write_ini(StrView iniPath, StrView section, StrView key, StrView val) {
	if (!WritePrivateProfileStringW(section.c_str(), key.c_str(), val.c_str(), iniPath.c_str())) [[unlikely]] {
		throw WinErr{GetLastError(), L"WritePrivateProfileString failed."};
	}
}


DirList::Iter& DirList::Iter::operator++() {
	if (_pDirList) {
		_pDirList->_advance();
		_check_finished_and_grab_wfd();
	}
	return *this;
}

bool DirList::Iter::operator==(const Iter &other) const noexcept {
	if (!_pDirList && !other._pDirList)
		return true;
	if (!_pDirList || !other._pDirList)
		return false;
	return _pDirList == other._pDirList;
}

DirList::Iter::Iter(DirList *pDirList)
	: _pDirList{pDirList}, _curFullPath{}
{
	if (_pDirList)
		_check_finished_and_grab_wfd(); // grab the first result immediately
}

void DirList::Iter::_check_finished_and_grab_wfd() {
	if (!_pDirList || !_pDirList->_hFind) { // search ended
		_pDirList = nullptr;
		_curFullPath.clear();
	} else {
		_curFullPath.reserve(MAX_PATH); // arbitrary
		_curFullPath = _pDirList->_basePath; // has trailing backslash
		_curFullPath += _pDirList->_wfd.cFileName;
	}
}


DirList::~DirList() {
	if (_hFind) {
		FindClose(_hFind);
		_hFind = nullptr;
	}
}

DirList::DirList(StrView dirPath)
	: _dirPath{dirPath}, _hFind{nullptr}, _wfd{}, _basePath{}
{
	static_assert(std::input_iterator<DirList::Iter>); // check our iterator integrity

	if (_dirPath.empty()) // from default ctor, will yield no results
		return;

	if (!file::is_dir(_dirPath)) [[unlikely]] {
		throw WinErr{ERROR_BAD_ARGUMENTS, str::fmt(L"Not a directory: %s", _dirPath)};
	}

	std::wstring userPath{_dirPath}; // copy
	if (!userPath.ends_with(L'\\'))
		userPath.push_back(L'\\');
	_basePath = userPath; // to concat the found filenames, ends in backslash
	userPath.push_back(L'*');

	_hFind = FindFirstFileW(userPath.c_str(), &_wfd);
	if (_hFind == INVALID_HANDLE_VALUE) {
		const DWORD err = GetLastError();
		if (err == ERROR_FILE_NOT_FOUND) [[likely]] { // no files found
			_hFind = nullptr;
		} else [[unlikely]] {
			throw WinErr{err, L"FindFirstFile failed."};
		}
	}

	StrView fileName{_wfd.cFileName};
	while ((fileName == L"." || fileName == L"..") && _hFind) // skip these
		_advance();
}

void DirList::_advance() {
	if (_hFind && !FindNextFileW(_hFind, &_wfd)) {
		const DWORD err = GetLastError();
		if (err == ERROR_NO_MORE_FILES) [[likely]] { // end of search
			FindClose(_hFind);
			_hFind = nullptr;
		} else [[unlikely]] {
			throw WinErr{err, L"FindNextFile failed."};
		}
	}
}


DirListDeep::Iter& DirListDeep::Iter::operator++() {
	if (_pDirListDeep) {
		_pDirListDeep->_advance();
		_check_finished_and_grab_wfd();
	}
	return *this;
}

bool DirListDeep::Iter::operator==(const Iter &other) const noexcept {
	if (!_pDirListDeep && !other._pDirListDeep)
		return true;
	if (!_pDirListDeep || !other._pDirListDeep)
		return false;
	return _pDirListDeep == other._pDirListDeep;
}

DirListDeep::Iter::Iter(DirListDeep *pDirListDeep)
	: _pDirListDeep{pDirListDeep}, _curFullPath{}
{
	if (_pDirListDeep)
		_check_finished_and_grab_wfd(); // grab the first result immediately
}

void DirListDeep::Iter::_check_finished_and_grab_wfd() {
	if (!_pDirListDeep || _pDirListDeep->_finished()) { // search ended
		_pDirListDeep = nullptr;
		_curFullPath.clear();
	} else {
		_curFullPath.reserve(MAX_PATH); // arbitrary
		_curFullPath = _pDirListDeep->_curFullPath();
	}
}


DirListDeep::~DirListDeep() noexcept {
	if (_pDeepIter)
		delete _pDeepIter;
	if (_pDeepList)
		delete _pDeepList;
}

DirListDeep::DirListDeep(StrView dirPath)
	: _flatList{dirPath}, _flatIter{_flatList.begin()}, _pDeepList{nullptr}, _pDeepIter{nullptr}
{
	_check_subfolder();
}

void DirListDeep::_check_subfolder() {
	const std::wstring &curFullPath = *_flatIter;
	if (file::is_dir(curFullPath)) {
		if (_pDeepList) {
			_pDeepIter->~Iter();
			_pDeepList->~DirListDeep();
			_pDeepList = new (_pDeepList) DirListDeep{curFullPath}; // reuse memory; recursively
			_pDeepIter = new (_pDeepIter) Iter{_pDeepList};
		} else {
			_pDeepList = new DirListDeep{curFullPath}; // recursively
			_pDeepIter = new DirListDeep::Iter{_pDeepList};
		}
	}
}

void DirListDeep::_advance() {
	if (_pDeepList) {
		++(*_pDeepIter);
		if (*_pDeepIter == DirListDeep{}.end()) { // deep search ended
			delete _pDeepIter; _pDeepIter = nullptr;
			delete _pDeepList; _pDeepList = nullptr;
			++_flatIter;
			_check_subfolder();
		}
	} else {
		++_flatIter;
		_check_subfolder();
	}
}


const IniFile::Entry* IniFile::Section::get(StrView keyName) const noexcept {
	for (auto &&entry : entries) {
		if (entry.key == keyName)
			return &entry;
	}
	return nullptr; // not found
}

std::wstring IniFile::Section::read_str(StrView keyName) const {
	const Entry *pEntry = get(keyName);
	if (!pEntry) [[unlikely]] {
		throw WinErr{ERROR_BAD_ARGUMENTS, L"Entry not found."};
	}
	return pEntry->val;
}

IniFile::Section& IniFile::Section::_raw_write(StrView keyName, std::wstring &&val) {
	if (Entry *pEntry = get(keyName); !pEntry) {
		entries.emplace_back(std::wstring{keyName}, std::move(val)); // insert new
	} else {
		pEntry->val = std::move(val);
	}
	return *this;
}

IniFile& IniFile::load(StrView iniPath) {
	const std::vector<BYTE> rawContents = file::read(iniPath);
	const std::wstring strContents = str::parse(rawContents);
	std::vector<std::wstring> lines = str::split_lines(strContents);

	Section curSection{};
	for (auto &&line : lines) {
		str::trim_spaces(line);
		if (line.empty() || line[0] == L'#' || line[0] == L';')
			continue; // skip blank and comment

		if (line[0] == L'[' && line.back() == L']') { // [section] ?
			if (!curSection.name.empty()) {
				sections.emplace_back(std::move(curSection));
				curSection = Section{};
			}
			curSection.name = line.substr(1, line.length() - 2); // begin new section
		} else if (!curSection.name.empty()) {
			std::vector<std::wstring> keyVal = str::split_n(line, L"=", 2);
			curSection.entries.emplace_back(std::move(keyVal[0]), std::move(keyVal[1])); // new entry
		}
	}

	if (!curSection.name.empty()) // for the last section
		sections.emplace_back(std::move(curSection));

	_iniPath = iniPath; // store path
	return *this;
}

IniFile& IniFile::save_as(StrView newIniPath, StrView lineBreak) {
	if (newIniPath.empty())
		throw WinErr{ERROR_BAD_ARGUMENTS, L"INI path not defined."};

	size_t sz = 0;
	for (auto &&sec : sections) { // 1st pass counts size
		sz += 1 + sec.name.length() + 1 + lineBreak.length();
		for (auto &&kv : sec.entries)
			sz += kv.key.length() + 1 + kv.val.length() + lineBreak.length();
		sz += lineBreak.length();
	}

	std::wstring outBuf{};
	outBuf.reserve(sz);

	for (auto &&sec : sections) { // 2nd pass copies to buffer
		outBuf.push_back(L'[');
		outBuf.append(sec.name);
		outBuf.push_back(L']');
		outBuf.append(lineBreak);

		for (auto &&kv : sec.entries) {
			outBuf.append(kv.key);
			outBuf.push_back(L'=');
			outBuf.append(kv.val);
			outBuf.append(lineBreak);
		}

		outBuf.append(lineBreak);
	}
	outBuf.resize(outBuf.length() - lineBreak.length()); // remove last double line break

	const std::vector<BYTE> outBlob = wd::str::to_utf8_blob(outBuf);
	file::write(newIniPath, outBlob);
	_iniPath = newIniPath; // save new path
	return *this;
}

const IniFile::Section* IniFile::get(StrView sectionName) const noexcept {
	for (auto &&section : sections) {
		if (section.name == sectionName)
			return &section;
	}
	return nullptr; // not found
}

const IniFile::Entry* IniFile::get(StrView sectionName, StrView keyName) const noexcept {
	const Section *pSection = get(sectionName);
	if (!pSection)
		return nullptr;
	return pSection->get(keyName);
}


static void version_load(VersionInfo &v, StrView exePath) {
	const DWORD sz = GetFileVersionInfoSizeW(exePath.c_str(), nullptr);
	if (!sz) [[unlikely]] {
		throw WinErr{GetLastError(), L"GetFileVersionInfoSize failed."};
	}

	std::vector<BYTE> data(sz, 0);
	if (BOOL ok = GetFileVersionInfoW(exePath.c_str(), 0, sz, data.data()); !ok) [[unlikely]] {
		throw WinErr{GetLastError(), L"GetFileVersionInfo failed."};
	}

	VS_FIXEDFILEINFO *pVsffi = nullptr;
#ifdef _MSC_VER
#pragma warning(suppress: 6387)
#endif
	if (BOOL ok = VerQueryValueW(data.data(), L"\\", reinterpret_cast<void**>(&pVsffi), nullptr); !ok) [[unlikely]] {
		throw WinErr{GetLastError(), L"VerQueryValue failed."};
	}
	v.version[0] = HIWORD(pVsffi->dwFileVersionMS);
	v.version[1] = LOWORD(pVsffi->dwFileVersionMS);
	v.version[2] = HIWORD(pVsffi->dwFileVersionLS);
	v.version[3] = LOWORD(pVsffi->dwFileVersionLS);

	struct Block final {
		LANGID langId = 0;
		WORD   codePage = 0;
	};
	Block *pBlock = nullptr;
	VerQueryValueW(data.data(), L"\\VarFileInfo\\Translation", reinterpret_cast<void**>(&pBlock), nullptr);

	std::initializer_list<std::pair<const WCHAR*, std::wstring*>> keysVals = {
		{L"Comments",         &v.comments},
		{L"CompanyName",      &v.companyName},
		{L"FileDescription",  &v.fileDescription},
		{L"FileVersion",      &v.fileVersion},
		{L"InternalName",     &v.internalName},
		{L"LegalCopyright",   &v.legalCopyright},
		{L"LegalTrademarks",  &v.legalTrademarks},
		{L"OriginalFilename", &v.originalFilename},
		{L"ProductName",      &v.productName},
		{L"ProductVersion",   &v.productVersion},
		{L"PrivateBuild",     &v.privateBuild},
		{L"SpecialBuild",     &v.specialBuild},
	};
	for (auto [key, pVal] : keysVals) {
		const std::wstring f = str::fmt(L"\\StringFileInfo\\%04x%04x\\%s", pBlock->langId, pBlock->codePage, key);
		WCHAR *pStr = nullptr;
		VerQueryValueW(data.data(), f.c_str(), reinterpret_cast<void**>(&pStr), nullptr);
		if (pStr)
			pVal->insert(0, pStr);
	}
}

VersionInfo::VersionInfo() {
	WCHAR exePath[MAX_PATH] = {0};
	GetModuleFileNameW(nullptr, exePath, ARRAYSIZE(exePath));
	version_load(*this, exePath);
}

VersionInfo::VersionInfo(StrView exePath) {
	version_load(*this, exePath);
}
