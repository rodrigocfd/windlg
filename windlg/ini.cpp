#include <system_error>
#include <Windows.h>
#include "ini.h"
#include "str.h"
using namespace lib;

UINT lib::ini::readInt(std::wstring_view iniPath, std::wstring_view section, std::wstring_view key)
{
	UINT val = GetPrivateProfileIntW(section.data(), key.data(), 0, iniPath.data());
	if (GetLastError() == ERROR_FILE_NOT_FOUND) [[unlikely]] {
		throw std::invalid_argument(str::toAnsi( str::fmt(L"INI entry not found: %s / %s", section, key) ));
	}
	return {val};
}

std::wstring lib::ini::readStr(std::wstring_view iniPath, std::wstring_view section, std::wstring_view key)
{
	UINT curBufSz = str::SSO_LEN;
	std::wstring buf(curBufSz, L'\0');

	for (;;) {
		GetPrivateProfileStringW(section.data(), key.data(), nullptr, buf.data(), curBufSz, iniPath.data());
		DWORD err = GetLastError();
		switch (err) {
		case ERROR_SUCCESS:
			str::trimNulls(buf);
			return {buf};
		case ERROR_MORE_DATA:
			curBufSz *= 2; // double the buffer size to try again
			buf.resize(curBufSz, L'\0');
			break;
		[[unlikely]] case ERROR_FILE_NOT_FOUND:
			throw std::invalid_argument(str::toAnsi( str::fmt(L"INI entry not found: %s / %s", section, key) ));
		[[unlikely]] default:
			throw std::system_error(err, std::system_category(), "GetPrivateProfileString failed");
		}
	}
}

void lib::ini::writeInt(std::wstring_view iniPath, std::wstring_view section, std::wstring_view key, UINT value)
{
	WCHAR buf[20] = {L'\0'};
	wsprintfW(buf, L"%d", value);
	writeStr(iniPath, section, key, buf);
}

void lib::ini::writeStr(std::wstring_view iniPath,
	std::wstring_view section, std::wstring_view key, std::wstring_view value)
{
	if (!WritePrivateProfileStringW(section.data(), key.data(), value.data(), iniPath.data())) [[unlikely]] {
		throw std::system_error(GetLastError(), std::system_category(), "WritePrivateProfileString failed");
	}
}
