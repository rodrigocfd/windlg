#pragma once
#include <optional>
#include <span>
#include <string>
#include <vector>
#include <Windows.h>

namespace lib::str {

constexpr size_t SSO_LEN = std::string{}.capacity();

[[nodiscard]] int cmp(std::wstring_view a, std::wstring_view b);
[[nodiscard]] int cmpI(std::wstring_view a, std::wstring_view b);
[[nodiscard]] bool contains(std::wstring_view s, std::wstring_view what, size_t off = 0);
[[nodiscard]] bool endsWith(std::wstring_view s, std::wstring_view theEnd);
[[nodiscard]] bool endsWithI(std::wstring_view s, std::wstring_view theEnd);
[[nodiscard]] bool eq(std::wstring_view a, std::wstring_view b);
[[nodiscard]] bool eqI(std::wstring_view a, std::wstring_view b);
[[nodiscard]] std::wstring fmtBytes(size_t numBytes);
[[nodiscard]] LPCWSTR guessLineBreak(std::wstring_view s);
[[nodiscard]] std::wstring join(std::span<std::wstring> all, std::wstring_view separator = L"");
[[nodiscard]] std::wstring newReserved(size_t numReserve);
[[nodiscard]] std::wstring newResized(size_t numResize, WCHAR ch = L'\0');
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

namespace _privfmt {
	template<typename T>
	[[nodiscard]] T fmtp(T val) {
		static_assert(!std::is_same_v<T, const char*>, "Non-wide char* being used on str::fmt(), str::toWide() can fix it.");
		static_assert(!std::is_same_v<T, std::string_view>, "Non-wide std::string_view being used on str::fmt(), str::toWide() can fix it.");
		static_assert(!std::is_same_v<T, std::string>, "Non-wide std::string being used on str::fmt(), str::toWide() can fix it.");
		return val;
	}
	[[nodiscard]] LPCWSTR fmtp(std::wstring_view val);
	[[nodiscard]] LPCWSTR fmtp(const std::wstring& val);
}

// String-safe wrapper to std::swprintf().
template<typename... T>
[[nodiscard]] std::wstring fmt(std::wstring_view format, const T&... args) {
	size_t len = std::swprintf(nullptr, 0, format.data(), _privfmt::fmtp(args)...);
	std::wstring buf(len + 1, L'\0'); // room for terminating null
	std::swprintf(buf.data(), len + 1, format.data(), _privfmt::fmtp(args)...);
	buf.resize(len); // remove terminating null
	return buf;
}

namespace enc {
	// Type of character encoding.
	enum class Type { Unknown, Ansi, Win1252, Utf8, Utf16be, Utf16le, Utf32be, Utf32le, Scsu, Bocu1 };

	// Character encoding information.
	struct Info final {
		Type encType = Type::Unknown;
		BYTE bomSize = 0;
	};

	[[nodiscard]] Info guess(std::span<BYTE> src);
}

}
