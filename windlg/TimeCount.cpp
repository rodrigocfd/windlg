#include <cmath>
#include <system_error>
#include "TimeCount.h"
using namespace lib;

LONGLONG TimeCount::_Freq = 0;

TimeCount TimeCount::Immediately()
{
	TimeCount t;
	t.restart();
	return t;
}

void TimeCount::restart()
{
	LARGE_INTEGER freq{}, t0{};
	if (!_Freq) {
		if (!QueryPerformanceFrequency(&freq)) [[unlikely]] {
			throw std::system_error(GetLastError(), std::system_category(), "QueryPerformanceFrequency failed");
		}
		_Freq = freq.QuadPart;
	}

	QueryPerformanceCounter(&t0);
	_t0 = t0.QuadPart;
}

TimeCount::Duration TimeCount::now() const
{
	LARGE_INTEGER t1{};
	QueryPerformanceCounter(&t1);
	auto ms = static_cast<LONGLONG>(std::round( (t1.QuadPart - _t0) / (_Freq / 1000.) ));

	Duration dur{};
	dur.ms = ms % 1000;
	dur.sec = static_cast<WORD>(((ms - dur.ms) / 1000) % 60);
	dur.min = static_cast<WORD>(((ms - dur.ms - dur.sec * 1000) / (60 * 1000)) % 60);
	dur.hr = static_cast<WORD>((ms - dur.ms - dur.sec * 1000 - dur.min * 60 * 1000) / (60 * 60 * 1000));
	return dur;
}
