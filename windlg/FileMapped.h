#pragma once
#include <span>
#include <string>
#include <vector>
#include "File.h"

namespace lib {

// Manages a memory-mapped file.
class FileMapped final {
public:
	enum class Access { ExistingReadOnly, ExistingRW };

	~FileMapped() { close(); }

	constexpr FileMapped() = default;
	FileMapped(const FileMapped&) = delete;
	FileMapped(FileMapped&& other) noexcept { operator=(std::forward<FileMapped>(other)); }
	FileMapped& operator=(const FileMapped&) = delete;
	FileMapped& operator=(FileMapped&& other) noexcept;

	FileMapped(std::wstring_view path, Access access) { open(path, access); }

	[[nodiscard]] const BYTE& operator[](size_t index) const { return reinterpret_cast<BYTE*>(_pMem)[index]; }
	[[nodiscard]] BYTE& operator[](size_t index) { return reinterpret_cast<BYTE*>(_pMem)[index]; }

	void close() noexcept;
	FileMapped& open(std::wstring_view path, Access access);
	[[nodiscard]] constexpr size_t size() const { return _sz; }
	[[nodiscard]] constexpr const std::span<BYTE> asSpan() const;
	[[nodiscard]] std::span<BYTE> asSpan();
	[[nodiscard]] constexpr const File& file() const { return _file; }

	[[nodiscard]] static std::vector<BYTE> ReadAll(std::wstring_view path);
	[[nodiscard]] static std::wstring ReadAllStr(std::wstring_view path);

private:
	File _file;
	HANDLE _hMap = nullptr;
	LPVOID _pMem = nullptr;
	size_t _sz = 0;
};

}
