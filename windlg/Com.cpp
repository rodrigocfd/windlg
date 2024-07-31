#include <system_error>
#include "Com.h"
using namespace lib;

void lib::checkHr(HRESULT hr, std::string_view funcName)
{
	if (FAILED(hr)) [[unlikely]] {
		std::string t{funcName};
		t.append(" failed");
		throw std::system_error(GetLastError(), std::system_category(), t);
	}
}


Com::~Com()
{
	CoUninitialize();
}

Com::Com(DWORD coInit)
{
	checkHr(CoInitializeEx(nullptr, coInit), "CoInitializeEx");
}


ComOle::~ComOle()
{
	OleUninitialize();
}

ComOle::ComOle()
{
	checkHr(OleInitialize(nullptr), "OleInitialize");
}
