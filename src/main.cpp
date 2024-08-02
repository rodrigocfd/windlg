#include <conio.h>
#include <iostream>
#include <windlg/lib.h>
using std::vector, std::wstring, std::wstring_view;

static auto dirLibWinDlg = L"D:\\Stuff\\Core\\cpp\\windlg\\windlg";
static auto baseDirTargets = L"D:\\Stuff\\Core\\cpp";
static auto targets = {
	L"flac-lame-frontend",
	L"id3-fit",
	L"king",
	L"upd-windlg",
	L"vscode-font-patch",
};

void processSolution(wstring_view target, wstring_view baseDirTargets, wstring_view dirLibWinDlg);
void processFilter(wstring_view target, wstring_view baseDirTargets, wstring_view dirLibWinDlg);

int main()
{
	for (auto&& target : targets) {
		std::wcout << L"Project " << target << L"..." << std::endl;
		processSolution(target, baseDirTargets, dirLibWinDlg);
		processFilter(target, baseDirTargets, dirLibWinDlg);
	}

	std::wcout << L"Press any key...";
	auto noop = _getch();
	return 0;
}
