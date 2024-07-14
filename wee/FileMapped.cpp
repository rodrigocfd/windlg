#include <system_error>
#include "FileMapped.h"
#include "str.h"
using namespace lib;

FileMapped& FileMapped::operator=(FileMapped&& other) noexcept
{
	close();
	std::swap(_file, other._file);
	std::swap(_hMap, other._hMap);
	std::swap(_pMem, other._pMem);
	std::swap(_sz, other._sz);
	return *this;
}

void FileMapped::close() noexcept
{
	_sz = 0;
	if (_pMem) {
		UnmapViewOfFile(_pMem);
		_pMem = nullptr;
	}
	if (_hMap) {
		CloseHandle(_hMap);
		_hMap = nullptr;
	}
	_file.close();
}

FileMapped& FileMapped::open(std::wstring_view path, Access access)
{
	close();
	File::Access facc = (access == Access::ExistingReadOnly) ? File::Access::ExistingRW : File::Access::ExistingRW;
	DWORD page = (access == Access::ExistingReadOnly) ? PAGE_READONLY : PAGE_READWRITE;

	_file.open(path, facc);

	_hMap = CreateFileMappingW(_file.hFile(), nullptr, page, 0, 0, nullptr);
	if (!_hMap) [[unlikely]] {
		throw std::system_error(GetLastError(), std::system_category(), "CreateFileMapping failed");
	}

	_pMem = MapViewOfFile(_hMap, FILE_MAP_READ | (access == Access::ExistingRW ? FILE_MAP_WRITE : 0), 0, 0, 0);
	if (!_pMem) [[unlikely]] {
		throw std::system_error(GetLastError(), std::system_category(), "MapViewOfFile failed");
	}

	_sz = _file.size();
	return *this;
}

constexpr const std::span<BYTE> FileMapped::asSpan() const
{
	return std::span{reinterpret_cast<BYTE*>(_pMem), size()};
}

std::span<BYTE> FileMapped::asSpan()
{
	return std::span{reinterpret_cast<BYTE*>(_pMem), size()};
}

std::vector<BYTE> FileMapped::ReadAll(std::wstring_view path)
{
	FileMapped f{path, Access::ExistingReadOnly};
	std::span<BYTE> raw = f.asSpan();
	return {raw.begin(), raw.end()};
}

std::wstring FileMapped::ReadAllStr(std::wstring_view path)
{
	FileMapped f{path, Access::ExistingReadOnly};
	return str::parse(f.asSpan());
}
