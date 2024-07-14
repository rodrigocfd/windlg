#include <system_error>
#include <Windows.h>
#include "ini.h"
#include "str.h"
using namespace lib;

std::optional<std::wstring> lib::ini::read(
	std::wstring_view iniPath, std::wstring_view section, std::wstring_view key)
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
			return std::nullopt;
		[[unlikely]] default:
			throw std::system_error(err, std::system_category(), "GetPrivateProfileString failed");
		}
	}
}

void lib::ini::write(std::wstring_view iniPath,
	std::wstring_view section, std::wstring_view key, std::wstring_view value)
{
	if (!WritePrivateProfileStringW(section.data(), key.data(), value.data(), iniPath.data())) [[unlikely]] {
		throw std::system_error(GetLastError(), std::system_category(), "WritePrivateProfileString failed");
	}
}
