#include <conio.h>
#include <iostream>
#include <windlg/lib.h>
using std::vector, std::wstring, std::wstring_view;
using namespace lib;

void processSolution(wstring_view target, wstring_view baseDirTargets, wstring_view dirLibWinDlg);
void processFilter(wstring_view target, wstring_view baseDirTargets, wstring_view dirLibWinDlg);

int main()
{
	Ini ini{path::exeDir() + L"\\upd-windlg.ini"};
	wstring pathLibWinDlg = ini.get(L"Paths", L"libWinDlg");
	wstring pathBaseTargets = ini.get(L"Paths", L"baseTargets");

	vector<wstring> programs;
	UINT i = 0;
	for (;;) {
		try {
			programs.emplace_back( ini.get(L"Programs", str::fmt(L"p%d", i++)) );
		} catch (const std::out_of_range&) {
			break;
		}
	}

	for (auto&& program : programs) {
		std::wcout << L"Updating " << program << L"..." << std::endl;
		processSolution(program, pathBaseTargets, pathLibWinDlg);
		processFilter(program, pathBaseTargets, pathLibWinDlg);
	}

	std::wcout << L"Press any key...";
	auto noop = _getch();
	return 0;
}
