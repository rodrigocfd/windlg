#include <algorithm>
#include "enc.h"
using namespace lib::enc;

constexpr bool _guessUtf8(std::span<BYTE> src);

Info lib::enc::guess(std::span<BYTE> src)
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
