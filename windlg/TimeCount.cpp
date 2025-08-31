#include <cmath>
#include <system_error>
#include "TimeCount.h"
using namespace lib;

static LONGLONG _freq = 0;

TimeCount TimeCount::Immediately()
{
	TimeCount t;
	t.restart();
	return t;
}

void TimeCount::restart()
{
	if (!_freq) { // frequency not cached yet?
		LARGE_INTEGER theFreq{};
		if (!QueryPerformanceFrequency(&theFreq)) [[unlikely]] {
			throw std::system_error(GetLastError(), std::system_category(), "QueryPerformanceFrequency failed");
		}
		_freq = theFreq.QuadPart; // cache frequency
	}

	LARGE_INTEGER t0;
	QueryPerformanceCounter(&t0);
	_t0 = t0.QuadPart;
}

TimeCount::Duration TimeCount::now() const
{
	LARGE_INTEGER t1{};
	QueryPerformanceCounter(&t1);
	auto ms = static_cast<LONGLONG>(std::round( (t1.QuadPart - _t0) / (_freq / 1000.) ));

	Duration dur{};
	dur.ms = ms % 1000;
	dur.sec = static_cast<WORD>(((ms - dur.ms) / 1000) % 60);
	dur.min = static_cast<WORD>(((ms - dur.ms - dur.sec * 1000) / (60 * 1000)) % 60);
	dur.hr = static_cast<WORD>((ms - dur.ms - dur.sec * 1000 - dur.min * 60 * 1000) / (60 * 60 * 1000));
	return dur;
}
