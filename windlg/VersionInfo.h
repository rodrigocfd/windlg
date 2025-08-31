#pragma once
#include <array>
#include <span>
#include <string_view>
#include <vector>
#include <Windows.h>

namespace lib {

// Loads version information from resource, from EXE or DLL file.
class VersionInfo final {
public:
	// Language ID and code page.
	struct LangCp final {
		WORD langId;
		WORD codePage;
	};

	VersionInfo();
	VersionInfo(const VersionInfo&) = delete;
	VersionInfo(VersionInfo&&) = delete;
	VersionInfo& operator=(const VersionInfo&) = delete;
	VersionInfo& operator=(VersionInfo&&) = delete;
	
	explicit VersionInfo(std::wstring_view exePath) { _load(exePath); }

	[[nodiscard]] constexpr std::span<const LangCp> langsCps() const { return _langsCps; }
	[[nodiscard]] std::wstring_view strInfo(LangCp langCp, std::wstring_view entryName) const;
	[[nodiscard]] const VS_FIXEDFILEINFO& verInfo() const;
	[[nodiscard]] std::array<WORD, 4> verNum() const;

private:
	void _load(std::wstring_view exePath);

	std::vector<BYTE> _data;
	std::span<const LangCp> _langsCps;
};

}
