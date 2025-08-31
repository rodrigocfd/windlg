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

	// Returns a new uninitialized TimeCount, you must call restart() to start counting.
	[[nodiscard]] constexpr static TimeCount Delayed() { return {}; }

	// Returns a new TimeCount which will start counting immediately.
	[[nodiscard]] static TimeCount Immediately();

	// Resets the counter.
	void restart();

	// Returns the duration since the timer was started.
	[[nodiscard]] Duration now() const;

private:
	constexpr TimeCount() = default;

	LONGLONG _t0 = 0;
};

}
