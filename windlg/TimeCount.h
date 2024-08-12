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

	constexpr TimeCount(const TimeCount&) = default;
	constexpr TimeCount(TimeCount&&) = default;
	constexpr TimeCount& operator=(const TimeCount&) = default;
	constexpr TimeCount& operator=(TimeCount&&) = default;

	[[nodiscard]] constexpr static TimeCount Delayed() { return {}; }
	[[nodiscard]] static TimeCount Immediately();

	void restart();
	[[nodiscard]] Duration now() const;

private:
	constexpr TimeCount() = default;

	static LONGLONG _Freq;
	LONGLONG _t0 = 0;
};

}
