#include <system_error>
#include "File.h"
#include "str.h"
using namespace lib;

File& File::operator=(File&& other) noexcept
{
	close();
	std::swap(_hFile, other._hFile);
	return *this;
}

void File::close() noexcept
{
	if (_hFile) {
		CloseHandle(_hFile);
		_hFile = nullptr;
	}
}

File& File::open(std::wstring_view path, Access access)
{
	close();
	DWORD acc = 0, share = 0, disp = 0;

	switch (access) {
	case Access::ExistingReadOnly:
		acc = GENERIC_READ;
		share = FILE_SHARE_READ;
		disp = OPEN_EXISTING;
		break;
	case Access::ExistingRW:
		acc = GENERIC_READ | GENERIC_WRITE;
		disp = OPEN_EXISTING;
		break;
	case Access::OpenOrCreateRW:
		acc = GENERIC_READ | GENERIC_WRITE;
		disp = OPEN_ALWAYS;
		break;
	case Access::CreateRW:
		acc = GENERIC_READ | GENERIC_WRITE;
		disp = CREATE_NEW;
	}

	_hFile = CreateFileW(path.data(), acc, share, nullptr, disp, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (!_hFile || _hFile == INVALID_HANDLE_VALUE) [[unlikely]] {
		throw std::system_error(GetLastError(), std::system_category(), "CreateFile failed");
	}
	return *this;
}

size_t File::pointerOffset() const
{
	LARGE_INTEGER zeroOffset{}, curOffset{};
	if (!SetFilePointerEx(_hFile, zeroOffset, &curOffset, FILE_CURRENT)) [[unlikely]] {
		throw std::system_error(GetLastError(), std::system_category(), "SetFilePointerEx failed");
	}
	return static_cast<size_t>(curOffset.QuadPart);
}

std::vector<BYTE> File::readAll() const
{
	setPointerOffset(0);
	std::vector<BYTE> buf(size(), 0x00);
	readBuffer(buf);
	return buf;
}

const File& File::readBuffer(std::vector<BYTE>& buffer) const
{
	DWORD read = 0;
	if (!ReadFile(_hFile, buffer.data(), static_cast<DWORD>(buffer.size()), &read, nullptr)) [[unlikely]] {
		throw std::system_error(GetLastError(), std::system_category(), "ReadFile failed");
	}
	return *this;
}

const File& File::setPointerOffset(size_t offset) const
{
	LARGE_INTEGER off = {.QuadPart = static_cast<LONGLONG>(offset)};
	if (!SetFilePointerEx(_hFile, off, nullptr, FILE_CURRENT)) [[unlikely]] {
		throw std::system_error(GetLastError(), std::system_category(), "SetFilePointerEx failed");
	}
	return *this;
}

const File& File::setSize(size_t newSizeBytes) const
{
	setPointerOffset(newSizeBytes);
	if (!SetEndOfFile(_hFile)) [[unlikely]] {
		throw std::system_error(GetLastError(), std::system_category(), "SetEndOfFile failed");
	}
	return *this;
}

size_t File::size() const
{
	LARGE_INTEGER sz{};
	if (!GetFileSizeEx(_hFile, &sz)) [[unlikely]] {
		throw std::system_error(GetLastError(), std::system_category(), "GetFileSizeEx failed");
	}
	return static_cast<size_t>(sz.QuadPart);
}

File::Times File::times() const
{
	FILETIME ftCreation{}, ftLastAccess{}, ftLastWrite{};
	if (!GetFileTime(_hFile, &ftCreation, &ftLastAccess, &ftLastWrite)) [[unlikely]] {
		throw std::system_error(GetLastError(), std::system_category(), "GetFileTime failed");
	}

	SYSTEMTIME stCreationUtc{}, stLastAccessUtc{}, stLastWriteUtc{};
	FileTimeToSystemTime(&ftCreation, &stCreationUtc);
	FileTimeToSystemTime(&ftLastAccess, &stLastAccessUtc);
	FileTimeToSystemTime(&ftLastWrite, &stLastWriteUtc);

	Times ret;
	SystemTimeToTzSpecificLocalTime(nullptr, &stCreationUtc, &ret.creation);
	SystemTimeToTzSpecificLocalTime(nullptr, &stLastAccessUtc, &ret.lastAccess);
	SystemTimeToTzSpecificLocalTime(nullptr, &stLastWriteUtc, &ret.lastWrite);
	
	return ret;
}

const File& File::write(std::span<BYTE> data) const
{
	DWORD written = 0;
	if (!WriteFile(_hFile, data.data(), static_cast<DWORD>(data.size_bytes()), &written, nullptr)) [[unlikely]] {
		throw std::system_error(GetLastError(), std::system_category(), "WriteFile failed");
	}
	return *this;
}

void File::EraseAndWrite(std::wstring_view path, std::span<BYTE> contents)
{
	File f{path, Access::OpenOrCreateRW};
	f.setSize(0); // sizing to contents was appending several zero bytes to the file content
	f.write(contents);
}

void File::EraseAndWriteStr(std::wstring_view path, std::wstring_view contents)
{
	std::vector<BYTE> raw{str::toUtf8Blob(contents)};
	EraseAndWrite(path, raw);
}
