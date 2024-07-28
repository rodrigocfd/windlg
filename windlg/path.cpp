#include <algorithm>
#include <system_error>
#include "path.h"
#include "str.h"
using namespace lib;
using namespace lib::path;

std::wstring lib::path::dirFrom(std::wstring_view p)
{
	std::wstring ret{p};
	size_t found = ret.find_last_of(L'\\'); // won't include trailing backslash
	if (found != std::wstring::npos)
		ret.resize(found);
	return ret;
}

std::vector<std::wstring> lib::path::dirList(std::wstring_view pathAndFilter)
{
	WIN32_FIND_DATAW wfd{};
	HANDLE hFind = FindFirstFileW(pathAndFilter.data(), &wfd);
	if (hFind == INVALID_HANDLE_VALUE) {
		DWORD err = GetLastError();
		FindClose(hFind);
		if (err == ERROR_FILE_NOT_FOUND) [[likely]] {
			return {}; // no files found
		} else [[unlikely]] {
			throw std::system_error(err, std::system_category(), "FindFirstFile failed");
		}
	}

	std::vector<std::wstring> entries;
	for (;;) {
		if (!str::eq(wfd.cFileName, L".") && !str::eq(wfd.cFileName, L"..")) { // skip these
			size_t idxBackslash = pathAndFilter.find_last_of(L"\\");
			if (idxBackslash != std::wstring::npos) [[likely]] {
				std::wstring fullPath{pathAndFilter.substr(0, idxBackslash + 1)};
				fullPath.append(wfd.cFileName);
				entries.emplace_back(std::move(fullPath));
			} else [[unlikely]] {
				std::wstring faultyPath{wfd.cFileName};
				throw std::logic_error("No backslash in found file " + str::toAnsi(faultyPath));
			}
		}

		if (!FindNextFileW(hFind, &wfd)) {
			DWORD err = GetLastError();
			FindClose(hFind);
			if (err == ERROR_NO_MORE_FILES) [[likely]] {
				std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) -> bool {
					return lstrcmpiW(a.c_str(), b.c_str()) < 1;
				});
				return entries; // no more files found
			} else [[unlikely]] {
				throw std::system_error(err, std::system_category(), "FindNextFile failed");
			}
		}
	}
}

void _dirWalkBuf(std::wstring_view pathAndFilter, std::vector<std::wstring>& outBuf)
{
	std::vector<std::wstring> entries = dirList(pathAndFilter);
	for (const auto& entry : entries) {
		if (!isDir(entry))
			outBuf.push_back(entry);
	}
	for (const auto& entry : entries) {
		if (isDir(entry)) {
			std::wstring subPath{entry};
			subPath.append(L"\\*");
			_dirWalkBuf(subPath, outBuf); // recursively, deep last
		}
	}
}

std::vector<std::wstring> lib::path::dirWalk(std::wstring_view pathAndFilter)
{
	std::vector<std::wstring> entries;
	_dirWalkBuf(pathAndFilter, entries);
	return entries;
}

std::wstring lib::path::exeDir()
{
	WCHAR buf[MAX_PATH] = {L'\0'};
	GetModuleFileNameW(nullptr, buf, ARRAYSIZE(buf));
	std::wstring p = dirFrom(buf);
#ifdef _DEBUG
	p = dirFrom(p);
#endif
	return p;
}

bool lib::path::exists(std::wstring_view p)
{
	DWORD attr = GetFileAttributesW(p.data());
	return attr != INVALID_FILE_ATTRIBUTES;
}

std::wstring lib::path::fileFrom(std::wstring_view p)
{
	std::wstring ret{p};
	size_t found = ret.find_last_of(L'\\');
	if (found != std::wstring::npos)
		ret.erase(0, found + 1);
	return ret;
}

bool lib::path::hasExtension(std::wstring_view p, std::initializer_list<std::wstring_view> exts)
{
	for (auto&& ext : exts) {
		if (str::endsWithI(p, ext))
			return true;
	}
	return false;
}

static DWORD _getAttrs(std::wstring_view p)
{
	DWORD attr = GetFileAttributesW(p.data());
	if (attr == INVALID_FILE_ATTRIBUTES) [[unlikely]] {
		throw std::system_error(GetLastError(), std::system_category(), "GetFileAttributes failed");
	}
	return attr;
}

bool lib::path::isDir(std::wstring_view p)
{
	return _getAttrs(p) & FILE_ATTRIBUTE_DIRECTORY;
}

bool lib::path::isHidden(std::wstring_view p)
{
	return _getAttrs(p) & FILE_ATTRIBUTE_HIDDEN;
}

bool lib::path::isReadOnly(std::wstring_view p)
{
	return _getAttrs(p) & FILE_ATTRIBUTE_READONLY;
}

std::wstring lib::path::swapExtension(std::wstring_view p, std::wstring newExt)
{
	size_t idxDot = p.find_last_of(L'.');
	if (idxDot != std::wstring::npos) [[likely]] {
		std::wstring p2{p.substr(0, idxDot + (newExt[0] == L'.' ? 0 : 1))};
		return p2 + newExt;
	} else [[unlikely]] {
		throw std::logic_error("File has no extension: " + str::toAnsi(p));
	}
}

void lib::path::trimBackslash(std::wstring& p)
{
	while (p.back() == L'\\')
		p.resize(p.length() - 1);
}
