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
[[nodiscard]] bool hasExtension(std::wstring_view p, std::wstring_view ext);
[[nodiscard]] bool hasExtension(std::wstring_view p, std::initializer_list<std::wstring_view> exts);
[[nodiscard]] bool isDir(std::wstring_view p);
[[nodiscard]] bool isHidden(std::wstring_view p);
[[nodiscard]] bool isReadOnly(std::wstring_view p);
[[nodiscard]] std::wstring swapExtension(std::wstring_view p, std::wstring newExt);
void trimBackslash(std::wstring& p);

}
