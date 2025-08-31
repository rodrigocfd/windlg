#include <system_error>
#include <Windows.h>
#pragma comment(lib, "Version.lib")
#include "VersionInfo.h"
using namespace lib;

VersionInfo::VersionInfo()
{
	WCHAR exePath[MAX_PATH] = {L'\0'};
	GetModuleFileNameW(nullptr, exePath, ARRAYSIZE(exePath));
	_load(exePath);
}

std::wstring_view VersionInfo::strInfo(LangCp langCp, std::wstring_view entryName) const
{
	WCHAR nfo[80] = {L'\0'};
	wsprintfW(nfo, L"\\StringFileInfo\\%04x%04x\\%s", langCp.langId, langCp.codePage, entryName.data());

	LPCWSTR pStr = nullptr;
	UINT szStr = 0;
	if (VerQueryValueW(_data.data(), nfo, reinterpret_cast<LPVOID*>(const_cast<LPWSTR*>(&pStr)), &szStr)) [[likely]] {
		return std::wstring_view{pStr, szStr - 1}; // don't count terminating null
	} else [[unlikely]] {
		throw std::logic_error("Version string doesn't exist");
	}
}

const VS_FIXEDFILEINFO& VersionInfo::verInfo() const
{
	const VS_FIXEDFILEINFO* pVi = nullptr;
	UINT szVi = 0;
	if (!VerQueryValueW(_data.data(), L"\\",
			reinterpret_cast<LPVOID*>(const_cast<VS_FIXEDFILEINFO**>(&pVi)), &szVi)) [[unlikely]] {
		throw std::runtime_error("VS_FIXEDFILEINFO not found");
	}
	return *pVi;
}

std::array<WORD, 4> VersionInfo::verNum() const
{
	const VS_FIXEDFILEINFO& vi = verInfo();
	return {HIWORD(vi.dwFileVersionMS), LOWORD(vi.dwFileVersionMS),
		HIWORD(vi.dwFileVersionLS), LOWORD(vi.dwFileVersionLS)};
}

void VersionInfo::_load(std::wstring_view exePath)
{
	UINT szData = GetFileVersionInfoSizeW(exePath.data(), nullptr);
	if (!szData) [[unlikely]] {
		throw std::system_error(GetLastError(), std::system_category(), "GetFileVersionInfoSize failed");
	}
	_data.resize(szData, 0x00); // alloc receiving buffer
	if (!GetFileVersionInfoW(exePath.data(), 0, szData, _data.data())) [[unlikely]] {
		throw std::system_error(GetLastError(), std::system_category(), "GetFileVersionInfo failed");
	}

	const LangCp* pLangCp = nullptr;
	UINT szBlock = 0;
	if (!VerQueryValueW(_data.data(), L"\\VarFileInfo\\Translation",
			reinterpret_cast<LPVOID*>(const_cast<LangCp**>(&pLangCp)), &szBlock)) [[unlikely]] {
		throw std::runtime_error("\\VarFileInfo\\Translation not found");
	}
	UINT numElems = szBlock / sizeof(LangCp);
	_langsCps = std::span{pLangCp, numElems}; // create a span over the receiving buffer
}
