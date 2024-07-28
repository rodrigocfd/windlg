#pragma once
#include <span>
#include <string_view>
#include <vector>
#include <Windows.h>

namespace lib {

// Manages a file HANDLE.
class File final {
public:
	// Requested access to open/create a file.
	enum class Access { ExistingReadOnly, ExistingRW, OpenOrCreateRW, CreateRW };

	// Returned by times().
	struct Times final {
		SYSTEMTIME creation{}, lastAccess{}, lastWrite{};
	};

private:
	HANDLE _hFile = nullptr;

public:
	~File() { close(); }

	constexpr File() = default;
	File(const File&) = delete;
	File(File&& other) noexcept { operator=(std::forward<File>(other)); }
	File& operator=(const File&) = delete;
	File& operator=(File&& other) noexcept;

	constexpr explicit File(HANDLE hFile) : _hFile{hFile} { }
	File(std::wstring_view path, Access access) { open(path, access); }

	void close() noexcept;
	[[nodiscard]] constexpr HANDLE hFile() const { return _hFile; }
	File& open(std::wstring_view path, Access access);
	[[nodiscard]] size_t pointerOffset() const;
	[[nodiscard]] std::vector<BYTE> readAll() const;
	const File& readBuffer(std::vector<BYTE>& buffer) const;
	const File& setPointerOffset(size_t offset) const;
	const File& setSize(size_t newSizeBytes) const;
	[[nodiscard]] size_t size() const;
	[[nodiscard]] Times times() const;
	const File& write(std::span<BYTE> data) const;

	static void EraseAndWrite(std::wstring_view path, std::span<BYTE> contents);
	static void EraseAndWriteStr(std::wstring_view path, std::wstring_view contents);
};

}
