#pragma once
#include "win-error.hpp"
#include "wnd.hpp"
#include <shtypes.h>

namespace wd {

	int run_cmd_sync(StrView cmdLine); // Calls CreateProcess() to run the command line synchronously.
	void run_thread_pool(size_t numRuns, std::function<void(size_t)> fun, size_t maxThreads = std::dynamic_extent); // Spawns threads and calls fun for numRuns times. This function is not reentrant.

	/// Measures a time interval.
	class StopWatch final {
	public:
		StopWatch() noexcept;
		StopWatch& restart() noexcept;       ///< Restarts the initial base time.
		size_t ellapsed_ms() const noexcept; ///< Returns the ellapsed time in milliseconds.
		std::wstring ellapsed_fmt() const;   ///< Returns the ellapsed time in milliseconds, conveniently formatted.
	private:
		LARGE_INTEGER _freq, _t0;
	};

}

/// Various system dialogs.
namespace wd::sys_dlg {

	void msg_ok(const BaseContainer *pOwner, StrView title, StrView body);
	void msg_warn(const BaseContainer *pOwner, StrView title, StrView body);
	void msg_err(const BaseContainer *pOwner, StrView title, StrView body);
	void msg_err(const BaseContainer *pOwner, StrView title, StrView body, const wd::WinErr &e);
	void msg_err(const BaseContainer *pOwner, StrView title, StrView body, const std::exception &e);
	[[nodiscard]] bool msg_ask(const BaseContainer *pOwner, StrView title, StrView body, StrView okBtn = L"OK");
	void msg_about(const BaseContainer *pOwner, WORD iconId = 0);

	[[nodiscard]] std::wstring file_open(const BaseContainer *pOwner, std::initializer_list<COMDLG_FILTERSPEC> filters);
	[[nodiscard]] std::vector<std::wstring> file_open_multi(const BaseContainer *pOwner, std::initializer_list<COMDLG_FILTERSPEC> filters);
	[[nodiscard]] std::wstring file_save(const BaseContainer *pOwner, std::initializer_list<COMDLG_FILTERSPEC> filters);
	[[nodiscard]] std::wstring folder_open(const BaseContainer *pOwner, StrView defFolder);

}

/// FILETIME utilities.
namespace wd::time {

	FILETIME& add_msec(FILETIME &ft, size_t numMsecs) noexcept;
	inline FILETIME& add_sec(FILETIME &ft, size_t numSecs) noexcept   { return add_msec(ft, numSecs * 1000); }
	inline FILETIME& add_min(FILETIME &ft, size_t numMins) noexcept   { return add_sec(ft, numMins * 60); }
	inline FILETIME& add_hour(FILETIME &ft, size_t numHours) noexcept { return add_sec(ft, numHours * 60); }
	inline FILETIME& add_day(FILETIME &ft, size_t numDays) noexcept   { return add_hour(ft, numDays * 24); }
	SYSTEMTIME& add_msec(SYSTEMTIME &st, size_t numMsecs) noexcept;
	inline SYSTEMTIME& add_sec(SYSTEMTIME &st, size_t numSecs) noexcept   { return add_msec(st, numSecs * 1000); }
	inline SYSTEMTIME& add_min(SYSTEMTIME &st, size_t numMins) noexcept   { return add_sec(st, numMins * 60); }
	inline SYSTEMTIME& add_hour(SYSTEMTIME &st, size_t numHours) noexcept { return add_min(st, numHours * 60); }
	inline SYSTEMTIME& add_day(SYSTEMTIME &st, size_t numDays) noexcept   { return add_hour(st, numDays * 24); }
	[[nodiscard]] size_t diff_ms(const FILETIME &a, const FILETIME &b) noexcept;     ///< Calculates the difference in miliseconds between the two values.
	[[nodiscard]] size_t diff_ms(const SYSTEMTIME &a, const SYSTEMTIME &b) noexcept; ///< Calculates the difference in miliseconds between the two values.
	[[nodiscard]] FILETIME now() noexcept;
	[[nodiscard]] FILETIME to_filetime(const SYSTEMTIME &st) noexcept;
	[[nodiscard]] SYSTEMTIME to_systemtime(const FILETIME &ft) noexcept;

}

[[nodiscard]] inline bool operator==(const FILETIME &a, const FILETIME &b) noexcept                  { return a.dwLowDateTime == b.dwLowDateTime && a.dwHighDateTime == b.dwHighDateTime; }
[[nodiscard]] inline std::strong_ordering operator<=>(const FILETIME &a, const FILETIME &b) noexcept { return CompareFileTime(&a, &b) <=> 0; }
