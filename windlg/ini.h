#pragma once
#include <optional>
#include <string>
#include <vector>
#include <Windows.h>

namespace lib {

// Handles sections and entries of an INI file.
class Ini final {
public:
	class Section final {
	public:
		struct KeyVal final {
			std::wstring key;
			std::wstring val;
		};

		std::wstring name;
		std::vector<KeyVal> keysVals;
	};

	std::optional<std::wstring> iniPath;
	std::vector<Section> sections;

	constexpr Ini() = default;
	constexpr Ini(const Ini&) = default;
	constexpr Ini(Ini&&) = default;
	constexpr Ini& operator=(const Ini&) = default;
	constexpr Ini& operator=(Ini&&) = default;

	explicit Ini(std::wstring iniPath) { load(iniPath); }

	[[nodiscard]] const std::wstring& get(std::wstring_view section, std::wstring_view key) const;
	[[nodiscard]] int getInt(std::wstring_view section, std::wstring_view key) const;
	void load(std::optional<std::wstring_view> iniPath = std::nullopt);
	void save(std::wstring_view br = L"\r\n") const;
	void set(std::wstring_view section, std::wstring_view key, std::wstring_view val);
	void setInt(std::wstring_view section, std::wstring_view key, int val);
};

}
