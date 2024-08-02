#include <stdexcept>
#include <windlg/lib.h>

static std::vector<std::wstring> _refillSolution(
	const std::vector<std::wstring>& lines, std::wstring_view tagSuffix, std::wstring_view ext, std::wstring_view dirLibWinDlg)
{
	size_t idxLine0 = 0, idxLinePast = 0;
	for (size_t i = 0; i < lines.size(); ++i) {
		if (!idxLine0) { // still searching the beginning of block
			if (lib::str::startsWith(lines[i], lib::str::fmt(L"    <Cl%s Include=\"..\\windlg\\windlg\\", tagSuffix))
					&& lib::str::endsWith(lines[i], lib::str::fmt(L".%s\" />", ext)) ) {
				idxLine0 = i;
			}
		} else { // now searching first line after block
			if (!lib::str::startsWith(lines[i], lib::str::fmt(L"    <Cl%s Include=\"..\\windlg\\windlg\\", tagSuffix))) {
				idxLinePast = i;
				break;
			}
		}
	}

	if (!idxLine0 || !idxLinePast)
		throw std::runtime_error("Solution line not found");

	std::vector<std::wstring> newLines{lines.begin(), lines.begin() + idxLine0};
	for (const auto& srcFile : lib::path::dirList( lib::str::fmt(L"%s\\*.%s", dirLibWinDlg, ext) )) {
		newLines.emplace_back(lib::str::fmt(L"    <Cl%s Include=\"..\\windlg\\windlg\\%s\" />",
			tagSuffix, lib::path::fileFrom(srcFile)));
	}
	newLines.insert(newLines.end(), lines.begin() + idxLinePast, lines.end());
	return newLines;
}

void processSolution(std::wstring_view target, std::wstring_view baseDirTargets, std::wstring_view dirLibWinDlg)
{
	auto solutionPath = lib::str::fmt(L"%s\\%s\\%s.vcxproj", baseDirTargets, target, target);
	auto lines = lib::str::splitLines(lib::FileMapped::ReadAllStr(solutionPath));

	lines = _refillSolution(lines, L"Compile", L"cpp", dirLibWinDlg);
	lines = _refillSolution(lines, L"Include", L"h", dirLibWinDlg);
	lib::File::EraseAndWriteStr(solutionPath, lib::str::join(lines, L"\r\n"));
}


static std::vector<std::wstring> _refillFilter(
	const std::vector<std::wstring>& lines, std::wstring_view tagSuffix, std::wstring_view ext, std::wstring_view dirLibWinDlg)
{
	size_t idxLine0 = 0, idxLinePast = 0;
	for (size_t i = 0; i < lines.size(); ++i) {
		if (!idxLine0) { // still searching the beginning of block
			if (lib::str::startsWith(lines[i], lib::str::fmt(L"    <Cl%s Include=\"..\\windlg\\windlg\\", tagSuffix))
					&& lib::str::endsWith(lines[i], lib::str::fmt(L".%s\">", ext)) ) {
				idxLine0 = i;
			}
		} else { // now searching first line after block
			i += 2; // will encompass 3 lines
			if (!lib::str::startsWith(lines[i], lib::str::fmt(L"    <Cl%s Include=\"..\\windlg\\windlg\\", tagSuffix))) {
				idxLinePast = i;
				break;
			}
		}
	}

	if (!idxLine0 || !idxLinePast)
		throw std::runtime_error("Filter line not found");

	std::vector<std::wstring> newLines{lines.begin(), lines.begin() + idxLine0};
	for (const auto& srcFile : lib::path::dirList( lib::str::fmt(L"%s\\*.%s", dirLibWinDlg, ext) )) {
		newLines.emplace_back(lib::str::fmt(L"    <Cl%s Include=\"..\\windlg\\windlg\\%s\">",
			tagSuffix, lib::path::fileFrom(srcFile)));
		newLines.emplace_back(L"      <Filter>WinDlg</Filter>");
		newLines.emplace_back(lib::str::fmt(L"    </Cl%s>", tagSuffix));
	}
	newLines.insert(newLines.end(), lines.begin() + idxLinePast, lines.end());
	return newLines;
}

void processFilter(std::wstring_view target, std::wstring_view baseDirTargets, std::wstring_view dirLibWinDlg)
{
	auto filterPath = lib::str::fmt(L"%s\\%s\\%s.vcxproj.filters", baseDirTargets, target, target);
	auto lines = lib::str::splitLines(lib::FileMapped::ReadAllStr(filterPath));

	lines = _refillFilter(lines, L"Compile", L"cpp", dirLibWinDlg);
	lines = _refillFilter(lines, L"Include", L"h", dirLibWinDlg);
	lib::File::EraseAndWriteStr(filterPath, lib::str::join(lines, L"\r\n"));
}
