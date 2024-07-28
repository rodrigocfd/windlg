#pragma once
#include <string>
#include <Windows.h>

namespace lib::ini {

[[nodiscard]] UINT readInt(std::wstring_view iniPath, std::wstring_view section, std::wstring_view key);
[[nodiscard]] std::wstring readStr(std::wstring_view iniPath, std::wstring_view section, std::wstring_view key);
void writeInt(std::wstring_view iniPath, std::wstring_view section, std::wstring_view key, UINT value);
void writeStr(std::wstring_view iniPath, std::wstring_view section, std::wstring_view key, std::wstring_view value);

}
