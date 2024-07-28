#pragma once
#include <span>
#include <Windows.h>

namespace lib::enc {

// Type of character encoding.
enum class Type { Unknown, Ansi, Win1252, Utf8, Utf16be, Utf16le, Utf32be, Utf32le, Scsu, Bocu1 };

// Character encoding information.
struct Info final {
	Type encType = Type::Unknown;
	BYTE bomSize = 0;
};

[[nodiscard]] Info guess(std::span<BYTE> src);

}
