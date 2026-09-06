#include <system_error>
#include "string.hpp"
#include "win-error.hpp"
#include <combaseapi.h>
using namespace wd;

CoString::~CoString() noexcept {
	if (_p) {
		CoTaskMemFree(_p);
		_p = nullptr;
	}
}

CoString& CoString::operator=(CoString &&other) noexcept {
	this->~CoString();
	std::swap(_p, other._p);
	return *this;
}

CoString& CoString::alloc(const WCHAR *s) {
	this->~CoString();
	_p = reinterpret_cast<WCHAR*>(CoTaskMemAlloc(sizeof(WCHAR) * (lstrlenW(s) + 1)));
	if (!_p) [[unlikely]] {
		throw WinErr{E_OUTOFMEMORY, L"CoTaskMemAlloc failed."};
	}
	lstrcpyW(_p, s);
	return *this;
}


std::vector<std::wstring> StrView::split(WCHAR delim) const {
	WCHAR delimBuf[2] = {0};
	delimBuf[0] = delim;
	return this->split(delimBuf);
}

std::vector<std::wstring> StrView::split_lines() const {
	const WCHAR *lineBreak = nullptr; // default: unknown
	for (size_t i = 0; i < this->length(); ++i) { // guess linebreak
		if ((*this)[i] == L'\r') {
			lineBreak = ((*this)[i + 1] == L'\n') ? L"\r\n" : L"\r"; // report the first one
			break;
		} else if ((*this)[i] == L'\n') {
			lineBreak = ((*this)[i + 1] == L'\r') ? L"\n\r" : L"\n";
			break;
		}
	}

	if (!lineBreak) {
		std::vector<std::wstring> arr{};
		arr.emplace_back(this->begin(), this->end()); // just 1 line
		return arr;
	}

	return split(lineBreak);
}

std::vector<std::wstring> StrView::split_n(StrView delim, size_t maxSubstrs) const {
	if (this->empty() || !maxSubstrs)
		return {};

	const size_t lenDelim = delim.length();
	if (!lenDelim)
		return {std::wstring{*this}}; // one single element

	size_t count = 1, iHead = 0;
	while (count < maxSubstrs) { // 1st pass counts the occurrences to prealloc; benchmarks proved it's about 2.7x faster
		iHead = this->find(delim.c_str(), iHead);
		if (iHead == StrView::npos)
			break; // not found
		++count;
		iHead += lenDelim; // now points to 1st char after delimiter
	}

	std::vector<std::wstring> subs{};
	subs.reserve(count); // prealloc the number of substrings

	size_t iBase = iHead = 0;
	for (size_t i = 0; i < count - 1; ++i) { // 2nd pass will extract and append the substrings
		iHead = this->find(delim.c_str(), iHead);
		if (iHead == StrView::npos)
			break; // not found
		subs.emplace_back(); // append empty string to vector
		subs.back().insert(0, *this, iBase, iHead - iBase); // insert chars into last appended string
		iHead += lenDelim; // now points to 1st char after delimiter
		iBase = iHead;
	}

	subs.emplace_back();
	subs.back().insert(0, *this, iBase, this->length() - iBase); // append the rest
	return subs;
}

std::wstring StrView::substr(size_t offset, size_t nChars) const {
	if (this->empty() || !nChars) [[unlikely]] {
		return {}; // nothing to copy
	}

	if (offset > this->length()) [[unlikely]] {
		offset = this->length();
	}

	if (nChars > this->length() - offset) [[unlikely]] {
		nChars = this->length() - offset; // protect from going beyond length
	}

	return std::wstring{this->c_str() + offset, nChars};
}

std::string StrView::to_ansi() const {
	std::string ansi(this->length(), '\0');
	for (size_t i = 0; i < this->length(); ++i)
		ansi[i] = static_cast<char>((*this)[i]); // brute-force conversion
	return ansi;
}

std::wstring StrView::to_lower() const {
	std::wstring cloned{*this};
	CharLowerW(cloned.data());
	return cloned;
}

std::wstring StrView::to_upper() const {
	std::wstring cloned{*this};
	CharUpperW(cloned.data());
	return cloned;
}

std::vector<BYTE> StrView::to_utf8_blob(bool writeBom) const {
	std::vector<BYTE> buf{};
	if (!this->empty()) {
		constexpr BYTE utf8bom[] = {0xef, 0xbb, 0xbf};
		const size_t szBom = writeBom ? ARRAYSIZE(utf8bom) : 0; // zero if we won't write the BOM

		const size_t neededLen = WideCharToMultiByte(CP_UTF8, 0,
			this->c_str(), static_cast<int>(this->length()), nullptr, 0, nullptr, 0);
		buf.resize(neededLen + szBom);

		if (writeBom)
			memcpy(buf.data(), utf8bom, szBom);

		WideCharToMultiByte(CP_UTF8, 0, this->c_str(), static_cast<int>(this->length()),
			reinterpret_cast<char*>(buf.data() + szBom),
			static_cast<int>(neededLen), nullptr, nullptr);
	}
	return buf;
}

constexpr static bool starts_ends_first_check(StrView s, StrView part) noexcept {
	if (part.empty())
		return true;
	if (s.empty() || part.length() > s.length())
		return false;
	return true;
}

static WCHAR single_char_upper(WCHAR ch) noexcept {
	return LOWORD(CharUpperW(reinterpret_cast<WCHAR*>(static_cast<UINT_PTR>(ch))));
}

bool StrView::starts_with(std::initializer_list<StrView> prefixes) const noexcept {
	for (auto &&prefix : prefixes) {
		if (operator std::wstring_view().starts_with(prefix.operator std::wstring_view()))
			return true;
	}
	return false;
}

bool StrView::ends_with(std::initializer_list<StrView> suffixes) const noexcept {
	for (auto &&suffix : suffixes) {
		if (!starts_ends_first_check(*this, suffix))
			continue;

		if (!lstrcmpW(this->c_str() + this->length() - suffix.length(), suffix.c_str()))
			return true;
	}
	return false;
}

bool StrView::starts_with_i(std::initializer_list<StrView> prefixes) const {
	std::wstring usUpper = this->to_upper();

	std::wstring prefixUpper{}; // buffer for all prefixes
	size_t lenMax = 0;
	for (auto &&prefix : prefixes) {
		if (prefix.length() > lenMax)
			lenMax = prefix.length(); // find the largest prefix
	}
	prefixUpper.reserve(lenMax);

	for (auto &&prefix : prefixes) {
		prefixUpper.clear();
		prefixUpper.insert(prefixUpper.begin(), prefix.begin(), prefix.end());
		str::to_upper(prefixUpper);
		if (str::starts_with(usUpper, prefixUpper))
			return true;
	}
	return false;
}

bool StrView::starts_with_i(WCHAR chPrefix) const noexcept {
	if (this->empty())
		return false;
	return single_char_upper(this->front()) == single_char_upper(chPrefix);
}

bool StrView::ends_with_i(std::initializer_list<StrView> suffixes) const noexcept {
	for (auto &&suffix : suffixes) {
		if (!starts_ends_first_check(*this, suffix))
			continue;

		if (!lstrcmpiW(this->c_str() + this->length() - suffix.length(), suffix.c_str()))
			return true;
	}
	return false;
}

bool StrView::ends_with_i(WCHAR chSuffix) const noexcept {
	if (this->empty())
		return false;
	return single_char_upper(this->back()) == single_char_upper(chSuffix);
}

std::wstring wd::operator+(StrView a, StrView b) {
	std::wstring s{};
	s.reserve(a.length() + b.length());
	s.append(a);
	s.append(b);
	return s;
}


std::wstring wd::str::fmt_bytes(ULONGLONG numBytes) {
	const ULONGLONG KB = 1024;
	const ULONGLONG MB = 1024 * KB;
	const ULONGLONG GB = 1024 * MB;
	const ULONGLONG TB = 1024 * GB;
	const ULONGLONG PB = 1024 * TB;
	const ULONGLONG EB = 1024 * PB;

	if      (numBytes < KB) return fmt(L"%u bytes", numBytes);
	else if (numBytes < MB) return fmt(L"%.2f KB", static_cast<double>(numBytes) / KB);
	else if (numBytes < GB) return fmt(L"%.2f MB", static_cast<double>(numBytes) / MB);
	else if (numBytes < TB) return fmt(L"%.2f GB", static_cast<double>(numBytes) / GB);
	else if (numBytes < PB) return fmt(L"%.2f TB", static_cast<double>(numBytes) / TB);
	else if (numBytes < EB) return fmt(L"%.2f PB", static_cast<double>(numBytes) / PB);
	else                    return fmt(L"%.2f EB", static_cast<double>(numBytes) / EB);
}

enum class Encoding { unknown, ansi, win_1252, utf8, utf16_be, utf16_le, utf32_be, utf32_le, scsu, bocu1 };

static constexpr bool guess_utf8(std::span<const BYTE> blob) noexcept {
	std::span<const BYTE>::iterator p = blob.begin(); // https://stackoverflow.com/a/1031773/6923555
	while (p != blob.end() && *p) {
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

static std::pair<Encoding, size_t> guess_encoding(std::span<const BYTE> blob) noexcept {
	auto match = [&blob](std::span<BYTE> bom) constexpr noexcept -> bool {
		return (blob.size() >= bom.size())
			&& std::equal(blob.begin(), blob.begin() + bom.size(), bom.begin(), bom.end());
	};

	BYTE utf8[] = {0xef, 0xbb, 0xbf}; // UTF-8 BOM
	if (match(utf8))
		return std::pair{Encoding::utf8, ARRAYSIZE(utf8)}; // BOM size in bytes

	BYTE utf16be[] = {0xfe, 0xff};
	if (match(utf16be))
		return std::pair{Encoding::utf16_be, ARRAYSIZE(utf16be)};

	BYTE utf16le[] = {0xff, 0xfe};
	if (match(utf16le))
		return std::pair{Encoding::utf16_le, ARRAYSIZE(utf16le)};

	BYTE utf32be[] = {0x00, 0x00, 0xfe, 0xff};
	if (match(utf32be))
		return std::pair{Encoding::utf32_be, ARRAYSIZE(utf32be)};

	BYTE utf32le[] = {0xff, 0xfe, 0x00, 0x00};
	if (match(utf32le))
		return std::pair{Encoding::utf32_le, ARRAYSIZE(utf32le)};

	BYTE scsu[] = {0x0e, 0xfe, 0xff};
	if (match(scsu))
		return std::pair{Encoding::scsu, ARRAYSIZE(scsu)};

	BYTE bocu1[] = {0xfb, 0xee, 0x28};
	if (match(bocu1))
		return std::pair{Encoding::bocu1, ARRAYSIZE(bocu1)};

	if (guess_utf8(blob))
		return std::pair{Encoding::utf8, 0}; // UTF-8 without BOM

	bool hasNonAnsiChar = false;
	for (auto &&ch : blob) {
		if (ch > 0x7f) {
			hasNonAnsiChar = true;
			break;
		}
	}

	return hasNonAnsiChar
		? std::pair{Encoding::win_1252, 0} // by exclusion, not assertive
		: std::pair{Encoding::ansi, 0};
}

static std::wstring parse_ansi(std::span<const BYTE> blob) {
	std::wstring ret{};
	if (!blob.empty()) {
		ret.resize(static_cast<int>(blob.size()));
		for (int i = 0; i < static_cast<int>(blob.size()); ++i) {
			if (blob[i] == 0x00) { // found terminating null
				ret.resize(i);
				return ret;
			}
			ret[i] = static_cast<WCHAR>(blob[i]); // brute-force conversion
		}
	}
	return ret; // data didn't have a terminating null
}

static std::wstring parse_encoded(std::span<const BYTE> blob, UINT codePage) {
	std::wstring ret{};
	if (!blob.empty()) {
		const int neededLen = MultiByteToWideChar(codePage, 0,
			reinterpret_cast<const char*>(blob.data()), static_cast<int>(blob.size()), nullptr, 0);
		ret.resize(neededLen);
		MultiByteToWideChar(codePage, 0, reinterpret_cast<const char*>(blob.data()),
			static_cast<int>(blob.size()), &ret[0], neededLen);
	}
	return ret;
}

static std::wstring parse_utf16(std::span<const BYTE> blob, bool isLE) {
	std::span<const WORD> wsrc{reinterpret_cast<const WORD*>(blob.data()), blob.size() / 2}; // will discard an odd byte
	std::wstring ret{};
	ret.reserve(static_cast<int>(wsrc.size()));
	for (auto &&ch : wsrc)
		ret.push_back(isLE ? ch : MAKEWORD(HIBYTE(ch), LOBYTE(ch)));
	return ret;
}

std::wstring wd::str::parse(std::span<const BYTE> blob) {
	if (blob.empty())
		return {};

	auto [enc, szBom] = guess_encoding(blob);
	blob = blob.subspan(szBom); // skip BOM, if any

	switch (enc) {
	using enum Encoding;
		case unknown:
		case ansi:     return parse_ansi(blob);
		case win_1252: return parse_encoded(blob, 1252);
		case utf8:     return parse_encoded(blob, CP_UTF8);
		case utf16_be: return parse_utf16(blob, false);
		case utf16_le: return parse_utf16(blob, true);
		case utf32_be: throw WinErr{E_NOTIMPL, L"UTF-32 big endian: encoding not implemented."};
		case utf32_le: throw WinErr{E_NOTIMPL, L"UTF-32 little endian: encoding not implemented."};
		case scsu:     throw WinErr{E_NOTIMPL, L"Standard compression scheme for Unicode: encoding not implemented."};
		case bocu1:    throw WinErr{E_NOTIMPL, L"Binary ordered compression for Unicode: encoding not implemented."};
		default:       throw WinErr{ERROR_NOT_FOUND, L"Unknown encoding."};
	}
}

std::wstring& wd::str::remove_diacritics(std::wstring &s) noexcept {
	const WCHAR *diacritics   = L"ÁáÀàÃãÂâÄäÉéÈèÊêËëÍíÌìÎîÏïÓóÒòÕõÔôÖöÚúÙùÛûÜüÇçÅåÐðÑñØøÝýÿ";
	const WCHAR *replacements = L"AaAaAaAaAaEeEeEeEeIiIiIiIiOoOoOoOoOoUuUuUuUuCcAaDdNnOoYyy";

	for (auto &&ch : s) {
		const WCHAR *pDiac = diacritics;
		const WCHAR *pRepl = replacements;
		while (*pDiac) {
			if (ch == *pDiac)
				ch = *pRepl; // in-place replacement
			++pDiac;
			++pRepl;
		}
	}
	return s;
}

std::wstring& wd::str::replace_all(std::wstring &s, StrView what, StrView replacement) {
	if (s.empty() || what.empty()) [[unlikely]] {
		return s;
	}

	if (replacement.length() > what.length()) { // we're growing
		size_t idx = 0, count = 0;
		while ((idx = s.find(what.c_str(), idx)) != std::wstring::npos) {
			++count;
			idx += what.length();
		}
		s.reserve(s.length() + count * (replacement.length() - what.length())); // prealloc
	}

	size_t idx = 0;
	while ((idx = s.find(what.c_str(), idx)) != std::wstring::npos) {
		s.replace(idx, what.length(), replacement.c_str());
		idx += replacement.length();
	}

	return s;
}

std::wstring& wd::str::to_lower(std::wstring &s) noexcept {
	CharLowerW(s.data());
	return s;
}

std::wstring& wd::str::to_upper(std::wstring &s) noexcept {
	CharUpperW(s.data());
	return s;
}

std::wstring wd::str::to_wide(const char *s) {
	if (!s)
		return std::wstring{};

	const int sLen = lstrlenA(s);
	std::wstring wide(sLen, L'\0');
	for (size_t i = 0; i < static_cast<size_t>(sLen); ++i)
		wide[i] = s[i]; // brute-force conversion
	return wide;
}

std::wstring& wd::str::trim_left(std::wstring &s, WCHAR charToTrim) {
	WCHAR charBuf[2] = {0};
	charBuf[0] = charToTrim;
	return trim_left(s, charBuf);
}

std::wstring& wd::str::trim_left(std::wstring &s, StrView charsToTrim) {
	size_t offset = 0;
	for (auto &&ch : s) {
		bool foundChar = false;
		for (auto &&chTrim : charsToTrim) {
			if (ch == chTrim) {
				++offset;
				foundChar = true;
				break;
			}
		}

		if (!foundChar)
			break;
	}

	for (size_t i = 0; i < s.length() - offset; ++i)
		s[i] = s[i + offset]; // move chars back
	s.resize(s.length() - offset);
	return s;
}

std::wstring& wd::str::trim_right(std::wstring &s, WCHAR charToTrim) {
	WCHAR charBuf[2] = {0};
	charBuf[0] = charToTrim;
	return trim_right(s, charBuf);
}

std::wstring& wd::str::trim_right(std::wstring &s, StrView charsToTrim) {
	size_t lastIdx = s.length();
	for (size_t i = s.length(); i-- > 0; ) {
		bool foundChar = false;
		for (auto &&chTrim : charsToTrim) {
			if (s[i] == chTrim) {
				lastIdx = i;
				foundChar = true;
				break;
			}
		}

		if (!foundChar)
			break;
	}

	s.resize(lastIdx);
	return s;
}

std::wstring& wd::str::trim_spaces(std::wstring &s) {
	const WCHAR *spaces = L" \f\n\r\t\v"; // https://en.cppreference.com/w/cpp/string/byte/isspace.html
	return trim_left(trim_right(s, spaces), spaces);
}

std::wstring wd::str::fmt_time(const FILETIME &ft, wd::str::Time format) {
	SYSTEMTIME st{};
	FileTimeToSystemTime(&ft, &st);
	return fmt_time(st, format);
}

std::wstring wd::str::fmt_time(const SYSTEMTIME &st, wd::str::Time format) {
	constexpr const WCHAR* week[] = {L"Sun", L"Mon", L"Tue", L"Wed", L"Thu", L"Fri", L"Sat"};
	switch (format) {
	using enum Time;
		case ymd:       return fmt(L"%04u-%02u-%02u", st.wYear, st.wMonth, st.wDay);
		case ymd_w:     return fmt(L"%04u-%02u-%02u %s", st.wYear, st.wMonth, st.wDay, week[st.wDayOfWeek]);
		case ymd_hm:    return fmt(L"%04u-%02u-%02u %02u:%02u", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
		case ymd_hms:   return fmt(L"%04u-%02u-%02u %02u:%02u:%02u", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		case ymd_hmsm:  return fmt(L"%04u-%02u-%02u %02u:%02u:%02u.%03u", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
		case hm:        return fmt(L"%02u:%02u", st.wHour, st.wMinute);
		case hms:       return fmt(L"%02u:%02u:%02u", st.wHour, st.wMinute, st.wSecond);
		case hmsm:      return fmt(L"%02u:%02u:%02u.%03u", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
		case sm:        return fmt(L"%02u.%03u", st.wSecond, st.wMilliseconds);
		default: [[unlikely]]
			throw WinErr{ERROR_BAD_ARGUMENTS, L"Unknown time format."}; // should never happen
	}
}
