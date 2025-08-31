#pragma once
#include <string>
#include <vector>
#include <Windows.h>

namespace lib::path {

// Returns the full directory of p, without the trailing backslash.
[[nodiscard]] std::wstring dirFrom(std::wstring_view p);

// Returns all files and folders within pathAndFilter, like "C:\\Temp\\*.mp3" or "C:\\Temp\\*".
[[nodiscard]] std::vector<std::wstring> dirList(std::wstring_view pathAndFilter);

// Returns, recursively on folders, all files within pathAndFilter, like "C:\\Temp\\*.mp3" or "C:\\Temp\\*".
[[nodiscard]] std::vector<std::wstring> dirWalk(std::wstring_view pathAndFilter);

// Returns the path of the current executable. In debug mode, goes up another level, returning the project path.
[[nodiscard]] std::wstring exeDir();

// Returns true if the folder or file exists.
[[nodiscard]] bool exists(std::wstring_view p);

// Returns the file name from p.
[[nodiscard]] std::wstring fileFrom(std::wstring_view p);

// Returns true if the p has the ext extension, case-insensitive.
[[nodiscard]] bool hasExtension(std::wstring_view p, std::wstring_view ext);

// Returns true if the p has one of the ext extensions, case-insensitive.
[[nodiscard]] bool hasExtension(std::wstring_view p, std::initializer_list<std::wstring_view> exts);

// Returns true if p is a directory.
[[nodiscard]] bool isDir(std::wstring_view p);

// Returns true if p is hidden.
[[nodiscard]] bool isHidden(std::wstring_view p);

// Returns true if p is read-only.
[[nodiscard]] bool isReadOnly(std::wstring_view p);

// Swaps the extension of p by newExt.
[[nodiscard]] std::wstring swapExtension(std::wstring_view p, std::wstring newExt);

// Trims any trailing backslashes, if any, in-place.
void trimBackslash(std::wstring& p);

}
