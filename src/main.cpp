#include <conio.h>
#include <iostream>
#include <windlg/lib.h>
using std::vector, std::wstring, std::wstring_view;

void processSolution(wstring_view target, wstring_view baseDirTargets, wstring_view dirLibWinDlg);
void processFilter(wstring_view target, wstring_view baseDirTargets, wstring_view dirLibWinDlg);

struct Config final {
	wstring pathLibWinDlg;
	wstring pathBaseTargets;
	vector<wstring> programs;
};

static Config _loadIniConfig()
{
	lib::Ini ini{lib::path::exeDir() + L"\\upd-windlg.ini"};

	Config c = {
		.pathLibWinDlg = ini.get(L"Paths", L"libWinDlg"),
		.pathBaseTargets = ini.get(L"Paths", L"baseTargets"),
	};

	UINT i = 0;
	for (;;) {
		try {
			c.programs.emplace_back( ini.get(L"Programs", lib::str::fmt(L"p%d", i++)) );
		} catch (const std::invalid_argument&) {
			break;
		}
	}
	return c;
}

int main()
{
	Config c = _loadIniConfig();
	for (auto&& program : c.programs) {
		std::wcout << L"Updating " << program << L"..." << std::endl;
		processSolution(program, c.pathBaseTargets, c.pathLibWinDlg);
		processFilter(program, c.pathBaseTargets, c.pathLibWinDlg);
	}

	std::wcout << L"Press any key...";
	auto noop = _getch();
	return 0;
}
