#include <system_error>
#include "Com.h"
using namespace lib;

Com::~Com()
{
	CoUninitialize();
}

Com::Com(DWORD coInit)
{
	_hr = CoInitializeEx(nullptr, coInit);
	Check(_hr, "CoInitializeEx");
}

void Com::Check(HRESULT hr, std::string_view funcName)
{
	if (FAILED(hr)) [[unlikely]] {
		std::string t{funcName};
		t.append(" failed");
		throw std::system_error(GetLastError(), std::system_category(), t);
	}
}
