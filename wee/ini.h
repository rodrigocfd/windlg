#pragma once
#include <optional>
#include <string>

namespace lib::ini {

[[nodiscard]] std::optional<std::wstring> read(std::wstring_view iniPath, std::wstring_view section, std::wstring_view key);
void write(std::wstring_view iniPath, std::wstring_view section, std::wstring_view key, std::wstring_view value);

}
