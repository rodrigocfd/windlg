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
	static void EraseAndWriteLines(std::wstring_view path, std::vector<std::wstring> lines, LPCWSTR br = L"\r\n");

private:
	HANDLE _hFile = nullptr;
};


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
	[[nodiscard]] static std::vector<std::wstring> ReadAllLines(std::wstring_view path);

private:
	File _file;
	HANDLE _hMap = nullptr;
	LPVOID _pMem = nullptr;
	size_t _sz = 0;
};

}
