#pragma once
#include <optional>
#include <span>
#include <string>
#include <vector>
#include <Windows.h>

namespace lib::str {

constexpr size_t SSO_LEN = std::string{}.capacity();

[[nodiscard]] bool contains(std::wstring_view s, std::wstring_view what, size_t off = 0);
[[nodiscard]] bool endsWith(std::wstring_view s, std::wstring_view theEnd);
[[nodiscard]] bool endsWithI(std::wstring_view s, std::wstring_view theEnd);
[[nodiscard]] bool eq(std::wstring_view a, std::wstring_view b);
[[nodiscard]] bool eqI(std::wstring_view a, std::wstring_view b);
[[nodiscard]] std::wstring fmtBytes(size_t numBytes);
[[nodiscard]] LPCWSTR guessLineBreak(std::wstring_view s);
[[nodiscard]] std::wstring join(std::span<std::wstring> all, std::wstring_view separator = L"");
[[nodiscard]] std::wstring newReserved(size_t numReserve);
[[nodiscard]] std::wstring parse(std::span<BYTE> src);
[[nodiscard]] std::optional<size_t> position(std::wstring_view s, std::wstring_view what, size_t off = 0);
[[nodiscard]] std::optional<size_t> positionRev(std::wstring_view s, std::wstring_view what, size_t off = 0);
void removeDiacritics(std::wstring& s);
[[nodiscard]] std::vector<std::wstring> split(std::wstring_view s, std::wstring_view delimiter);
[[nodiscard]] std::vector<std::wstring> splitLines(std::wstring_view s);
[[nodiscard]] bool startsWith(std::wstring_view s, std::wstring_view theStart);
[[nodiscard]] bool startsWithI(std::wstring_view s, std::wstring_view theStart);
[[nodiscard]] std::string toAnsi(std::wstring_view s);
[[nodiscard]] std::wstring toLower(std::wstring_view s);
[[nodiscard]] std::wstring toUpper(std::wstring_view s);
[[nodiscard]] std::vector<BYTE> toUtf8Blob(std::wstring_view s, bool writeBom = false);
[[nodiscard]] std::wstring toWide(std::string_view s);
void trim(std::wstring& s);
void trimNulls(std::wstring& s);

template<size_t SZ>
std::wstring fmt(std::wstring_view format, ...) {
	WCHAR buf[SZ] = {L'\0'};
	va_list argPtr;
	va_start(argPtr, format);
	std::vswprintf(buf, ARRAYSIZE(buf), format.data(), argPtr);
	va_end(argPtr);
	return {buf};
}

}
