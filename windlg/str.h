#pragma once
#include <optional>
#include <span>
#include <string>
#include <vector>
#include <Windows.h>

namespace lib::str {

// Default number of characters of a string allocated with Short String Optimization.
constexpr size_t SSO_LEN = std::string{}.capacity();

// Calls lstrcmp() to compare the strings lexographically, case-sensitive.
[[nodiscard]] int cmp(std::wstring_view a, std::wstring_view b);

// Calls lstrcmpi() to compare the strings lexographically, case-insensitive.
[[nodiscard]] int cmpI(std::wstring_view a, std::wstring_view b);

// Returns true if s contains the substring what, starting from offset off.
[[nodiscard]] bool contains(std::wstring_view s, std::wstring_view what, size_t off = 0);

// Returns true if s ends with theStart, case-sensitive.
[[nodiscard]] bool endsWith(std::wstring_view s, std::wstring_view theEnd);

// Returns true if s ends with theStart, case-insensitive.
[[nodiscard]] bool endsWithI(std::wstring_view s, std::wstring_view theEnd);

// Calls lstrcmp() for case-sensitive equality.
[[nodiscard]] bool eq(std::wstring_view a, std::wstring_view b);

// Calls lstrcmpi() for case-insensitive equality.
[[nodiscard]] bool eqI(std::wstring_view a, std::wstring_view b);

// Converts numBytes into a string with the highest unit, up to petabytes.
[[nodiscard]] std::wstring fmtBytes(size_t numBytes);

// Guesses the linebreak characters: CR, CRLF, LF or LFCR.
[[nodiscard]] LPCWSTR guessLineBreak(std::wstring_view s);

// Returns a new string by joining the strings in all with separator.
[[nodiscard]] std::wstring join(std::span<std::wstring> all, std::wstring_view separator = L"");

// Returns a new wstring with numReserve reserved chars.
[[nodiscard]] std::wstring newReserved(size_t numReserve);

// Returns a new wstring resized with numResize occurrences of ch.
[[nodiscard]] std::wstring newResized(size_t numResize, WCHAR ch = L'\0');

// Guesses the encoding and parses src into a wstring.
[[nodiscard]] std::wstring parse(std::span<BYTE> src);

// Returns the first occurrence of the substring what in s, if any.
// Starts searching from the offset off.
[[nodiscard]] std::optional<size_t> position(std::wstring_view s, std::wstring_view what, size_t off = 0);

// Returns the last occurrence of the substring what in s, if any.
// Starts searching from the offset off.
[[nodiscard]] std::optional<size_t> positionRev(std::wstring_view s, std::wstring_view what, size_t off = std::wstring::npos);

// Removes the diacritics from s, in-place.
void removeDiacritics(std::wstring& s);

// Returns a vector with substrings of s, delimited by delimiter.
[[nodiscard]] std::vector<std::wstring> split(std::wstring_view s, std::wstring_view delimiter);

// Returns a vector with each line of s as a string.
[[nodiscard]] std::vector<std::wstring> splitLines(std::wstring_view s);

// Returns true if s starts with theStart, case-sensitive.
[[nodiscard]] bool startsWith(std::wstring_view s, std::wstring_view theStart);

// Returns true if s starts with theStart, case-insensitive.
[[nodiscard]] bool startsWithI(std::wstring_view s, std::wstring_view theStart);

// Converts wstring to string. The inverse is done by toWide().
[[nodiscard]] std::string toAnsi(std::wstring_view s);

// Returns a new string, converted to lowercase.
[[nodiscard]] std::wstring toLower(std::wstring_view s);

// Returns a new string, converted to uppercase.
[[nodiscard]] std::wstring toUpper(std::wstring_view s);

// Converts s into UTF-8 bytes with WideCharToMultiByte().
[[nodiscard]] std::vector<BYTE> toUtf8Blob(std::wstring_view s, bool writeBom = false);

// Converts string to wstring. The inverse is done by toAnsi().
[[nodiscard]] std::wstring toWide(std::string_view s);

// Calls iswspace() to remove all spaces from beginning and end of the string.
// Also calls trimNulls().
void trim(std::wstring& s);

// Calls lstrlen() and resizes the wstring, so that its size() will match
// the actual string length, not counting any terminating nulls.
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

// String-safe wrapper to std::swprintf(), which also accepts wstring and wstring_view as arguments.
template<typename... T>
[[nodiscard]] std::wstring fmt(std::wstring_view format, const T&... args) {
	size_t len = std::swprintf(nullptr, 0, format.data(), _privfmt::fmtp(args)...);
	std::wstring buf(len + 1, L'\0'); // room for terminating null
	std::swprintf(buf.data(), len + 1, format.data(), _privfmt::fmtp(args)...);
	buf.resize(len); // remove terminating null
	return buf;
}

// Encoding-related operations.
namespace enc {
	// Type of character encoding.
	enum class Type { Unknown, Ansi, Win1252, Utf8, Utf16be, Utf16le, Utf32be, Utf32le, Scsu, Bocu1 };

	// Character encoding information.
	struct Info final {
		Type encType = Type::Unknown;
		BYTE bomSize = 0;
	};

	// Guesses the encoding of the given binary blob.
	[[nodiscard]] Info guess(std::span<BYTE> src);
}

}
