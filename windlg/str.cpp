#include <algorithm>
#include <cwctype>
#include <stdexcept>
#include <Windows.h>
#include "str.h"

int lib::str::cmp(std::wstring_view a, std::wstring_view b)
{
	return lstrcmpW(a.data(), b.data());
}

int lib::str::cmpI(std::wstring_view a, std::wstring_view b)
{
	return lstrcmpiW(a.data(), b.data());
}

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

bool lib::str::eq(std::wstring_view a, std::wstring_view b)
{
	return !lstrcmpW(a.data(), b.data());
}

bool lib::str::eqI(std::wstring_view a, std::wstring_view b)
{
	return !lstrcmpiW(a.data(), b.data());
}

std::wstring lib::str::fmtBytes(size_t numBytes)
{
	constexpr size_t sz = 40;
	if (numBytes < 1024)
		return fmt(L"%zu bytes", numBytes);
	else if (numBytes < 1024ull * 1024)
		return fmt(L"%.2f KB", numBytes / 1024.);
	else if (numBytes < 1024ull * 1024 * 1024)
		return fmt(L"%.2f MB", numBytes / 1024. / 1024.);
	else if (numBytes < 1024ull * 1024 * 1024 * 1024)
		return fmt(L"%.2f GB", numBytes / 1024. / 1024. / 1024.);
	else if (numBytes < 1024ull * 1024 * 1024 * 1024 * 1024)
		return fmt(L"%.2f TB", numBytes / 1024. / 1024. / 1024. / 1024.);
	else
		return fmt(L"%.2f PB", numBytes / 1024. / 1024. / 1024. / 1024. / 1024.);
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

std::wstring lib::str::newResized(size_t numResize, WCHAR ch)
{
	std::wstring s;
	s.resize(numResize, ch);
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

LPCWSTR lib::str::_privfmt::fmtp(std::wstring_view val)
{
	return val.data();
}

LPCWSTR lib::str::_privfmt::fmtp(const std::wstring& val)
{
	return val.c_str();
}


static constexpr bool _guessUtf8(std::span<BYTE> src)
{
	std::span<BYTE>::iterator p = src.begin(); // https://stackoverflow.com/a/1031773/6923555
	while (p != src.end() && *p) {
		if ( // ASCII
			// use p[0] <= 0x7f to allow ASCII control characters
			p[0] == 0x09 ||
			p[0] == 0x0a ||
			p[0] == 0x0d ||
			(0x20 <= p[0] && p[0] <= 0x7e)
		) {
			std::advance(p, 1);
			continue;
		}

		if ( // non-overlong 2-byte
			(0xc2 <= p[0] && p[0] <= 0xdf) &&
			(0x80 <= p[1] && p[1] <= 0xbf)
		) {
			std::advance(p, 2);
			continue;
		}

		if (( // excluding overlongs
			p[0] == 0xe0 &&
			(0xa0 <= p[1] && p[1] <= 0xbf) &&
			(0x80 <= p[2] && p[2] <= 0xbf)
		) || ( // straight 3-byte
			((0xe1 <= p[0] && p[0] <= 0xec) ||
				p[0] == 0xee ||
				p[0] == 0xef) &&
			(0x80 <= p[1] && p[1] <= 0xbf) &&
			(0x80 <= p[2] && p[2] <= 0xbf)
		) || ( // excluding surrogates
			p[0] == 0xed &&
			(0x80 <= p[1] && p[1] <= 0x9f) &&
			(0x80 <= p[2] && p[2] <= 0xbf)
		)) {
			std::advance(p, 3);
			continue;
		}

		if (( // planes 1-3
			p[0] == 0xf0 &&
			(0x90 <= p[1] && p[1] <= 0xbf) &&
			(0x80 <= p[2] && p[2] <= 0xbf) &&
			(0x80 <= p[3] && p[3] <= 0xbf)
		) || ( // planes 4-15
			(0xf1 <= p[0] && p[0] <= 0xf3) &&
			(0x80 <= p[1] && p[1] <= 0xbf) &&
			(0x80 <= p[2] && p[2] <= 0xbf) &&
			(0x80 <= p[3] && p[3] <= 0xbf)
		) || ( // plane 16
			p[0] == 0xf4 &&
			(0x80 <= p[1] && p[1] <= 0x8f) &&
			(0x80 <= p[2] && p[2] <= 0xbf) &&
			(0x80 <= p[3] && p[3] <= 0xbf)
		)) {
			std::advance(p, 4);
			continue;
		}

		return false; // none of the conditions were accepted, not UTF-8
	}
	return true; // all the conditions accepted through the whole byte source
}

lib::str::enc::Info lib::str::enc::guess(std::span<BYTE> src)
{
	auto match = [&](std::span<BYTE> bom) constexpr -> bool {
		return (src.size() >= bom.size())
			&& std::equal(src.begin(), src.end(), bom.begin(), bom.end());
	};

	BYTE utf8[] = {0xef, 0xbb, 0xbf}; // UTF-8 BOM
	if (match(utf8)) return {Type::Utf8, ARRAYSIZE(utf8)}; // BOM size in bytes

	BYTE utf16be[] = {0xfe, 0xff};
	if (match(utf16be)) return {Type::Utf16be, ARRAYSIZE(utf8)};

	BYTE utf16le[] = {0xff, 0xfe};
	if (match(utf16le)) return {Type::Utf16le, ARRAYSIZE(utf8)};

	BYTE utf32be[] = {0x00, 0x00, 0xfe, 0xff};
	if (match(utf32be)) return {Type::Utf32be, ARRAYSIZE(utf8)};

	BYTE utf32le[] = {0xff, 0xfe, 0x00, 0x00};
	if (match(utf32le)) return {Type::Utf32le, ARRAYSIZE(utf8)};

	BYTE scsu[] = {0x0e, 0xfe, 0xff};
	if (match(scsu)) return {Type::Scsu, ARRAYSIZE(utf8)};

	BYTE bocu1[] = {0xfb, 0xee, 0x28};
	if (match(bocu1)) return {Type::Bocu1, ARRAYSIZE(utf8)};

	if (_guessUtf8(src)) return {Type::Utf8, 0}; // UTF-8 without BOM

	bool hasNonAnsiChar = std::any_of(src.begin(), src.end(), [](BYTE ch) { return ch > 0x7f; });
	return hasNonAnsiChar
		? Info{Type::Win1252, 0} // by exclusion, not assertive
		: Info{Type::Ansi, 0};
}
