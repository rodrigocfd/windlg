#include <system_error>
#include <Windows.h>
#include "File.h"
#include "ini.h"
#include "str.h"
using namespace lib;

const std::wstring& Ini::get(std::wstring_view section, std::wstring_view key) const
{
	for (const Section& s : sections) {
		if (str::eq(section, s.name)) {
			for (const Section::KeyVal& kv : s.keysVals) {
				if (str::eq(key, kv.key))
					return kv.val;
			}
		}
	}
	throw std::out_of_range( str::toAnsi(str::fmt(L"Not found: %s / %s", section, key)) );
}

int Ini::getInt(std::wstring_view section, std::wstring_view key) const
{
	return std::stoi(get(section, key));
}

void Ini::load(std::optional<std::wstring_view> iniPath)
{
	if (!this->iniPath.has_value() && !iniPath.has_value())
		throw std::invalid_argument("No INI path provided for load");

	if (iniPath.has_value())
		this->iniPath = iniPath;

	Section curSection; // keys before the first section will be ignored
	std::vector<std::wstring> lines = FileMapped::ReadAllLines(this->iniPath.value());

	for (std::wstring& line : lines) {
		str::trim(line);
		if (line.empty()) { // skip blank lines
			continue;
		} else if (line[0] == L'[' && line.back() == L']') { // begin of section found
			if (!curSection.name.empty()) {
				sections.emplace_back(std::move(curSection)); // add previous section
				curSection = {}; // make static analysis happy
			}
			curSection.name.insert(0, &line[1], line.length() - 2); // extract section name
		} else if (!curSection.name.empty() && line[0] != L';' && line[0] != L'#') { // lines starting with ; or # will be ignored
			size_t idxEq = line.find_first_of(L'=');
			if (idxEq != std::wstring::npos) {
				Section::KeyVal keyVal;
				keyVal.key.insert(0, &line[0], idxEq); // extract key name
				keyVal.val.insert(0, &line[idxEq + 1], line.length() - (idxEq + 1)); // extract value
				curSection.keysVals.emplace_back(std::move(keyVal));
			}
		}
	}

	if (!curSection.name.empty())
		sections.emplace_back(std::move(curSection));
}

void Ini::save(std::wstring_view br) const
{
	if (!iniPath.has_value())
		throw std::invalid_argument("No INI path provided for save");

	size_t count = 0;
	for (const Section& s : sections) {
		count += 2 + s.name.length() + br.length(); // [section]
		for (const Section::KeyVal& kv : s.keysVals)
			count += kv.key.length() + 1 + kv.val.length() + br.length(); // key=val
		count += br.length();
	}

	std::wstring buf;
	buf.reserve(count);

	for (const Section& s : sections) {
		buf.push_back(L'[');
		buf.append(s.name);
		buf.push_back(L']');
		buf.append(br);

		for (const Section::KeyVal& kv : s.keysVals) {
			buf.append(kv.key);
			buf.push_back(L'=');
			buf.append(kv.val);
			buf.append(br);
		}
		buf.append(br);
	}

	buf.resize(buf.length() - br.length()); // remove the last br
	File::EraseAndWriteStr(iniPath.value(), buf);
}

void Ini::set(std::wstring_view section, std::wstring_view key, std::wstring_view val)
{
	for (Section& s : sections) {
		for (Section::KeyVal& kv : s.keysVals) {
			if (str::eq(key, kv.key)) {
				kv.val = val;
				return;
			}
		}
	}
	throw std::out_of_range( str::toAnsi(str::fmt(L"Not found: %s / %s", section, key)) );
}

void Ini::setInt(std::wstring_view section, std::wstring_view key, int val)
{
	set(section, key, std::to_wstring(val));
}
