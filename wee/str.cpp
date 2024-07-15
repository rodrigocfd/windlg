#include <cwctype>
#include <format>
#include <stdexcept>
#include <Windows.h>
#include "enc.h"
#include "str.h"

bool lib::str::contains(std::wstring_view s, std::wstring_view what, size_t off)
{
	return s.find(what, off) != std::wstring::npos;
}

bool lib::str::endsWith(std::wstring_view s, std::wstring_view theEnd)
{
	if (s.empty() || theEnd.empty() || theEnd.length() > s.length()) return false;
	return !lstrcmpW(s.data() + s.length() - theEnd.length(), theEnd.data());
}

bool lib::str::endsWithI(std::wstring_view s, std::wstring_view theEnd)
{
	if (s.empty() || theEnd.empty() || theEnd.length() > s.length()) return false;
	return !lstrcmpiW(s.data() + s.length() - theEnd.length(), theEnd.data());
}

std::wstring lib::str::fmtBytes(size_t numBytes)
{
	if (numBytes < 1024) {
		return std::format(L"{} bytes", numBytes);
	} else if (numBytes < 1024 * 1024) {
		double d = static_cast<double>(numBytes) / 1024;
		return std::format(L"{:.2f} KB", d);
	} else if (numBytes < 1024 * 1024 * 1024) {
		double d = static_cast<double>(numBytes) / 1024 / 1024;
		return std::format(L"{:.2f} MB", d);
	} else if (numBytes < 1024ull * 1024 * 1024 * 1024) {
		double d = static_cast<double>(numBytes) / 1024 / 1024 / 1024;
		return std::format(L"{:.2f} GB", d);
	} else if (numBytes < 1024ull * 1024 * 1024 * 1024 * 1024) {
		double d = static_cast<double>(numBytes) / 1024 / 1024 / 1024 / 1024;
		return std::format(L"{:.2f} TB", d);
	} else {
		double d = static_cast<double>(numBytes) / 1024 / 1024 / 1024 / 1024 / 1024;
		return std::format(L"{:.2f} PB", d);
	}
}

LPCWSTR lib::str::guessLineBreak(std::wstring_view s)
{
	for (size_t i = 0; i < s.length() - 1; ++i) {
		if (s[i] == L'\r') {
			return s[i + 1] == L'\n' ? L"\r\n" : L"\r"; // report the first one
		} else if (s[i] == L'\n') {
			return s[i + 1] == L'\r' ? L"\n\r" : L"\n";
		}
	}
	return nullptr; // unknown
}

std::wstring lib::str::join(std::span<std::wstring> all, std::wstring_view separator)
{
	std::wstring buf;
	bool first = true;
	for (const auto& s : all) {
		if (first) {
			first = false;
		} else {
			buf.append(separator);
		}
		buf.append(s);
	}
	return buf;
}

std::wstring lib::str::newReserved(size_t numReserve)
{
	std::wstring s;
	s.reserve(numReserve);
	return s;
}

std::optional<size_t> lib::str::position(std::wstring_view s, std::wstring_view what, size_t off)
{
	size_t pos = s.find(what, off);
	return pos == std::wstring::npos ? std::nullopt : std::optional{pos};
}

std::optional<size_t> lib::str::positionRev(std::wstring_view s, std::wstring_view what, size_t off)
{
	size_t pos = s.rfind(what, off);
	return pos == std::wstring::npos ? std::nullopt : std::optional{pos};
}

static std::wstring _parseAnsi(std::span<BYTE> src)
{
	std::wstring ret;
	if (!src.empty()) {
		ret.resize(src.size());
		for (size_t i = 0; i < src.size(); ++i) {
			if (src[i] == 0x00) { // found terminating null
				ret.resize(i);
				return ret;
			}
			ret[i] = static_cast<wchar_t>(src[i]); // brute-force conversion
		}
	}
	return ret; // data didn't have a terminating null
}

static std::wstring _parseEncoded(std::span<BYTE> src, UINT codePage)
{
	std::wstring ret;
	if (!src.empty()) {
		int neededLen = MultiByteToWideChar(codePage, 0,
			reinterpret_cast<const char*>(src.data()), static_cast<int>(src.size()), nullptr, 0);
		ret.resize(neededLen);
		MultiByteToWideChar(codePage, 0, reinterpret_cast<const char*>(src.data()),
			static_cast<int>(src.size()), &ret[0], neededLen);
		lib::str::trimNulls(ret);
	}
	return ret;
}

std::wstring lib::str::parse(std::span<BYTE> src)
{
	if (src.empty()) return {};

	enc::Info encInfo = enc::guess(src);
	src = src.subspan(encInfo.bomSize); // skip BOM, if any

	switch (encInfo.encType) {
	using enum enc::Type;
		case Unknown:
		case Ansi:    return _parseAnsi(src);
		case Win1252: return _parseEncoded(src, 1252);
		case Utf8:    return _parseEncoded(src, CP_UTF8);
		case Utf16be: throw std::invalid_argument("UTF-16 big endian: encoding not implemented.");
		case Utf16le: throw std::invalid_argument("UTF-16 little endian: encoding not implemented.");
		case Utf32be: throw std::invalid_argument("UTF-32 big endian: encoding not implemented.");
		case Utf32le: throw std::invalid_argument("UTF-32 little endian: encoding not implemented.");
		case Scsu:    throw std::invalid_argument("Standard compression scheme for Unicode: encoding not implemented.");
		case Bocu1:   throw std::invalid_argument("Binary ordered compression for Unicode: encoding not implemented.");
		default:      throw std::invalid_argument("Unknown encoding.");
	}
}

void lib::str::removeDiacritics(std::wstring& s)
{
	LPCWSTR diacritics   = L"¡·¿‡√„¬‚ƒ‰…È»Ë ÍÀÎÕÌÃÏŒÓœÔ”Û“Ú’ı‘Ù÷ˆ⁄˙Ÿ˘€˚‹¸«Á≈Â–—Òÿ¯›˝";
	LPCWSTR replacements = L"AaAaAaAaAaEeEeEeEeIiIiIiIiOoOoOoOoOoUuUuUuUuCcAaDdNnOoYy";

	for (WCHAR& ch : s) {
		LPCWSTR pDiac = diacritics;
		LPCWSTR pRepl = replacements;
		while (*pDiac) {
			if (ch == *pDiac) ch = *pRepl; // in-place replacement
			++pDiac;
			++pRepl;
		}
	}
}

std::vector<std::wstring> lib::str::split(std::wstring_view s, std::wstring_view delimiter)
{
	if (s.empty()) return {};
	if (delimiter.empty())
		return {std::wstring{s}}; // one single element

	std::vector<std::wstring> ret;
	size_t base = 0, head = 0;
	for (;;) {
		head = s.find(delimiter, head);
		if (head == std::wstring::npos) break;
		ret.emplace_back();
		ret.back().insert(0, s, base, head - base);
		head += lstrlenW(delimiter.data());
		base = head;
	}

	ret.emplace_back();
	ret.back().insert(0, s, base, s.length() - base);
	return ret;
}

std::vector<std::wstring> lib::str::splitLines(std::wstring_view s)
{
	return split(s, guessLineBreak(s));
}

bool lib::str::startsWith(std::wstring_view s, std::wstring_view theStart)
{
	if (s.empty() || theStart.empty() || theStart.length() > s.length()) return false;
	for (size_t i = 0; i < theStart.length(); ++i)
		if (s.data()[i] != theStart.data()[i]) return false;
	return true;
}

bool lib::str::startsWithI(std::wstring_view s, std::wstring_view theStart)
{
	if (s.empty() || theStart.empty() || theStart.length() > s.length()) return false;
	std::wstring s2{s};
	s2.resize(theStart.length());
	return !lstrcmpiW(s2.data(), theStart.data());
}

std::string lib::str::toAnsi(std::wstring_view s)
{
	std::string ansi(s.length(), '\0');
	for (size_t i = 0; i < s.length(); ++i) {
		ansi[i] = static_cast<char>(s[i]); // brute-force conversion
	}
	return ansi;
}

std::wstring lib::str::toLower(std::wstring_view s)
{
	std::wstring ret{s};
	CharLowerBuffW(ret.data(), static_cast<DWORD>(ret.length()));
	return ret;
}

std::wstring lib::str::toUpper(std::wstring_view s)
{
	std::wstring ret{s};
	CharUpperBuffW(ret.data(), static_cast<DWORD>(ret.length()));
	return ret;
}

std::vector<BYTE> lib::str::toUtf8Blob(std::wstring_view s, bool writeBom)
{
	std::vector<BYTE> buf;

	if (!s.empty()) {
		constexpr BYTE utf8bom[] = {0xef, 0xbb, 0xbf};
		size_t szBom = writeBom ? ARRAYSIZE(utf8bom) : 0;

		size_t neededLen = WideCharToMultiByte(CP_UTF8, 0,
			s.data(), static_cast<int>(s.length()),
			nullptr, 0, nullptr, 0);
		buf.resize(neededLen + szBom);

		if (writeBom)
			CopyMemory(buf.data(), utf8bom, szBom);

		WideCharToMultiByte(CP_UTF8, 0,
			s.data(), static_cast<int>(s.length()),
			reinterpret_cast<char*>(buf.data() + szBom),
			static_cast<int>(neededLen), nullptr, nullptr);
	}

	return buf;
}

std::wstring lib::str::toWide(std::string_view s)
{
	std::wstring wide(s.length(), L'\0');
	for (size_t i = 0; i < s.length(); ++i) {
		wide[i] = s[i]; // brute-force conversion
	}
	return wide;
}

void lib::str::trimNulls(std::wstring& s)
{
	// When a std::wstring is initialized with any length, possibly to be used as a buffer,
	// the string length may not match the size() method, after the operation.
	// This function fixes this.
	if (!s.empty())
		s.resize( lstrlenW(s.c_str()) );
}

void lib::str::trim(std::wstring& s)
{
	if (s.empty()) return;
	trimNulls(s);

	size_t len = s.length();
	size_t iFirst = 0, iLast = len - 1; // bounds of trimmed string
	bool onlySpaces = true; // our string has only spaces?

	for (size_t i = 0; i < len; ++i) {
		if (!std::iswspace(s[i])) {
			iFirst = i;
			onlySpaces = false;
			break;
		}
	}
	if (onlySpaces) {
		s.clear();
		return;
	}

	for (size_t i = len; i-- > 0; ) {
		if (!std::iswspace(s[i])) {
			iLast = i;
			break;
		}
	}

	std::copy(s.begin() + iFirst, // move the non-space chars back
		s.begin() + iLast + 1, s.begin());
	s.resize(iLast - iFirst + 1); // trim container size
}
