#pragma once
#include <Windows.h>

namespace lib {

// Measures a time interval.
class TimeCount final {
public:
	// Duration of a time interval.
	struct Duration final {
		WORD ms = 0;
		WORD sec = 0;
		WORD min = 0;
		WORD hr = 0;
	};

	constexpr TimeCount() = default;
	constexpr TimeCount(const TimeCount&) = default;
	constexpr TimeCount(TimeCount&&) = default;
	constexpr TimeCount& operator=(const TimeCount&) = default;
	constexpr TimeCount& operator=(TimeCount&&) = default;

	void start();
	[[nodiscard]] Duration now() const;

private:
	static LONGLONG _Freq;
	LONGLONG _t0 = 0;
};

}
