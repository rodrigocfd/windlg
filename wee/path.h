#pragma once
#include <string>
#include <vector>
#include <Windows.h>

namespace lib::path {

[[nodiscard]] std::wstring dirFrom(std::wstring_view p);
[[nodiscard]] std::vector<std::wstring> dirList(std::wstring_view pathAndFilter);
[[nodiscard]] std::vector<std::wstring> dirWalk(std::wstring_view pathAndFilter);
[[nodiscard]] std::wstring exeDir();
[[nodiscard]] bool exists(std::wstring_view p);
[[nodiscard]] std::wstring fileFrom(std::wstring_view p);
[[nodiscard]] bool isDir(std::wstring_view p);
[[nodiscard]] bool isHidden(std::wstring_view p);
[[nodiscard]] bool isReadOnly(std::wstring_view p);
void trimBackslash(std::wstring& p);

}
