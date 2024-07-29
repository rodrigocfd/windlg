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

private:
	static LONGLONG _Freq;
	LONGLONG _t0 = 0;

public:
	constexpr TimeCount() = default;
	constexpr TimeCount(const TimeCount&) = default;
	TimeCount(TimeCount&&) = delete;
	constexpr TimeCount& operator=(const TimeCount&) = default;
	TimeCount& operator=(TimeCount&&) = delete;

	void start();
	[[nodiscard]] Duration now() const;
};

}
