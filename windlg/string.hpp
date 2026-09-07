#pragma once
#include <span>
#include <string>
#include <vector>

#include <sdkddkver.h>
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace wd {

	/// A wide string pointer with CoTaskMemAlloc() and CoTaskMemFree().
	class CoString final {
	public:
		~CoString() noexcept;
		constexpr CoString() noexcept                  : _p{nullptr} { }
		constexpr explicit CoString(WCHAR *p) noexcept : _p{p} { }

		constexpr CoString(CoString &&other) noexcept : _p{other._p} { other._p = nullptr; }
		CoString& operator=(CoString &&other) noexcept;

		[[nodiscard]] size_t length() const noexcept                       { return lstrlenW(_p); }
		[[nodiscard]] constexpr WCHAR* c_str() const noexcept              { return _p; }
		[[nodiscard]] constexpr WCHAR operator[](int index) const noexcept { return _p[index]; }
		[[nodiscard]] constexpr WCHAR** pptr() noexcept                    { return &_p; }
		CoString& alloc(const WCHAR *s);

	private:
		WCHAR *_p;
	};

	/// A null-terminated std::wstring_view.
	class StrView final {
	public:
		using value_type = WCHAR;
		using size_type = size_t;
		static constexpr size_t npos = std::wstring_view::npos;

		constexpr StrView(const std::wstring &s) noexcept    : _ptr{s.c_str()}, _len{s.size()} { }
		constexpr StrView(const WCHAR *p, size_t n) noexcept : _ptr{p}, _len{n} { }
		StrView(const WCHAR *p)                              : _ptr{p}, _len{static_cast<size_t>(lstrlenW(p))} { }
		StrView(const CoString &s)                           : _ptr{s.c_str()}, _len{s.length()} { }

		StrView(StrView&&) = default;
		StrView(const StrView&) = default;
		StrView& operator=(StrView&&) = default;
		StrView& operator=(const StrView&) = default;

		[[nodiscard]] constexpr operator std::wstring_view() const noexcept { return std::wstring_view{_ptr, _len}; } // Implicit conversion to std::wstring_view.
		[[nodiscard]] WCHAR operator[](size_t index) const noexcept         { return _ptr[index]; }
		[[nodiscard]] constexpr WCHAR back() const noexcept                 { return _ptr[_len - 1]; } // Returns last char.
		[[nodiscard]] constexpr const WCHAR* c_str() const noexcept         { return _ptr; }
		[[nodiscard]] constexpr int compare(StrView other) const noexcept   { return operator std::wstring_view().compare(other.operator std::wstring_view()); }
		[[nodiscard]] int compare_i(StrView other) const noexcept           { return lstrcmpiW(c_str(), other.c_str()); }
		[[nodiscard]] constexpr bool contains(StrView s) const noexcept     { return find(s) != npos; }
		[[nodiscard]] constexpr bool empty() const noexcept                 { return !_len; }
		[[nodiscard]] bool eq_i(StrView other) const noexcept               { return !compare_i(other); }
		[[nodiscard]] constexpr WCHAR front() const noexcept                { return _ptr[0]; } // Returns first char.
		[[nodiscard]] constexpr size_t length() const noexcept              { return _len; }
		[[nodiscard]] bool neq_i(StrView other) const noexcept              { return !eq_i(other); }
		[[nodiscard]] std::vector<std::wstring> split(WCHAR delim) const;
		[[nodiscard]] std::vector<std::wstring> split(StrView delim) const  { return split_n(delim, std::dynamic_extent); }
		[[nodiscard]] std::vector<std::wstring> split_lines() const;
		[[nodiscard]] std::vector<std::wstring> split_n(StrView delim, size_t maxSubstrs) const;
		[[nodiscard]] std::wstring substr(size_t offset, size_t nChars = std::dynamic_extent) const;
		[[nodiscard]] std::string to_ansi() const; // Converts a wide string into an ANSI string.
		[[nodiscard]] std::wstring to_lower() const;
		[[nodiscard]] std::wstring to_upper() const;
		[[nodiscard]] std::vector<BYTE> to_utf8_blob(bool writeBom = false) const;

		[[nodiscard]] constexpr size_t find(StrView what, size_t off = 0) const noexcept                { return operator std::wstring_view().find(what.operator std::wstring_view(), off); }
		[[nodiscard]] constexpr size_t find_first_not_of(StrView what, size_t off = 0) const noexcept   { return operator std::wstring_view().find_first_not_of(what.operator std::wstring_view(), off); }
		[[nodiscard]] constexpr size_t find_first_not_of(WCHAR ch, size_t off = 0) const noexcept       { return operator std::wstring_view().find_first_not_of(ch, off); }
		[[nodiscard]] constexpr size_t find_first_of(StrView what, size_t off = 0) const noexcept       { return operator std::wstring_view().find_first_of(what.operator std::wstring_view(), off); }
		[[nodiscard]] constexpr size_t find_first_of(WCHAR ch, size_t off = 0) const noexcept           { return operator std::wstring_view().find_first_of(ch, off); }
		[[nodiscard]] constexpr size_t find_last_not_of(StrView what, size_t off = npos) const noexcept { return operator std::wstring_view().find_last_not_of(what.operator std::wstring_view(), off); }
		[[nodiscard]] constexpr size_t find_last_not_of(WCHAR ch, size_t off = npos) const noexcept     { return operator std::wstring_view().find_last_not_of(ch, off); }
		[[nodiscard]] constexpr size_t find_last_of(StrView what, size_t off = npos) const noexcept     { return operator std::wstring_view().find_last_of(what.operator std::wstring_view(), off); }
		[[nodiscard]] constexpr size_t find_last_of(WCHAR ch, size_t off = npos) const noexcept         { return operator std::wstring_view().find_last_of(ch, off); }
		[[nodiscard]] constexpr size_t rfind(StrView what, size_t off = npos) const noexcept            { return operator std::wstring_view().rfind(what.operator std::wstring_view(), off); }

		[[nodiscard]] bool starts_with(std::initializer_list<StrView> prefixes) const noexcept;
		[[nodiscard]] constexpr bool starts_with(StrView prefix) const noexcept { return operator std::wstring_view().starts_with(prefix.operator std::wstring_view()); }
		[[nodiscard]] constexpr bool starts_with(WCHAR chPrefix) const noexcept { return operator std::wstring_view().starts_with(chPrefix); }
		[[nodiscard]] bool ends_with(std::initializer_list<StrView> suffixes) const noexcept;
		[[nodiscard]] constexpr bool ends_with(StrView suffix) const noexcept   { return operator std::wstring_view().ends_with(suffix.operator std::wstring_view()); }
		[[nodiscard]] constexpr bool ends_with(WCHAR chSuffix) const noexcept   { return operator std::wstring_view().ends_with(chSuffix); }
		[[nodiscard]] bool starts_with_i(std::initializer_list<StrView> prefixes) const;
		[[nodiscard]] bool starts_with_i(StrView prefix) const                  { return starts_with_i(std::initializer_list{prefix}); }
		[[nodiscard]] bool starts_with_i(WCHAR chPrefix) const noexcept;
		[[nodiscard]] bool ends_with_i(std::initializer_list<StrView> suffixes) const noexcept;
		[[nodiscard]] bool ends_with_i(StrView suffix) const noexcept           { return ends_with_i(std::initializer_list{suffix}); }
		[[nodiscard]] bool ends_with_i(WCHAR chSuffix) const noexcept;

		[[nodiscard]] constexpr const WCHAR* begin() const noexcept { return _ptr; } // Iterator begin().
		[[nodiscard]] constexpr const WCHAR* end() const noexcept   { return _ptr + _len; } // Iterator end().
		[[nodiscard]] constexpr auto rbegin() const noexcept        { return std::reverse_iterator(end()); } // Iterator rbegin().
		[[nodiscard]] constexpr auto rend() const noexcept          { return std::reverse_iterator(begin()); } // Iterator rend().

		[[nodiscard]] friend constexpr bool operator==(StrView a, StrView b) noexcept                  { return a.operator std::wstring_view() == b.operator std::wstring_view(); }
		[[nodiscard]] friend constexpr std::strong_ordering operator<=>(StrView a, StrView b) noexcept { return a.operator std::wstring_view() <=> b.operator std::wstring_view(); }
		friend std::wstring operator+(StrView a, StrView b);

	private:
		const WCHAR *_ptr;
		size_t _len; // not counting terminating null
	};

}

namespace _wd_internal {

	template<typename T>
	concept IsCharPointer =
		std::is_pointer_v<std::remove_reference_t<T>> &&
		std::same_as<
			std::remove_cv_t<std::remove_pointer_t<std::remove_reference_t<T>>>,
			char>;
	template<typename T>
	concept IsCharArray =
		std::is_array_v<std::remove_reference_t<T>> &&
		std::same_as<
			std::remove_cv_t<std::remove_extent_t<std::remove_reference_t<T>>>,
			char>;
	template<typename T>
	concept IsStdCharClass =
		std::same_as<std::remove_cvref_t<T>, std::string> ||
		std::same_as<std::remove_cvref_t<T>, std::string_view>;
	template<typename T>
	concept IsCharType = IsCharPointer<T> || IsCharArray<T> || IsStdCharClass<T>;

	template<typename T>
	constexpr decltype(auto) fmt_arg(T &&arg) { // used internally by str::fmt
		static_assert(!IsCharType<T>, "ERROR: str::fmt only wide strings are allowed.");
		using U = std::remove_cvref_t<T>;
		static_assert(!std::same_as<U, std::wstring_view>, "ERROR: str::fmt doesn't allow std::wstring_view.");
		if constexpr (std::same_as<U, wd::StrView> || std::same_as<U, std::wstring>) {
			return arg.c_str();
		} else {
			return std::forward<T>(arg);
		}
	}

}

/// String utilities.
namespace wd::str {

	// String formatting capable of handling std::wstring and wd::StrView.
	template<typename ...Args>
	[[nodiscard]] std::wstring fmt(wd::StrView format, Args &&...args) {
		int nChars = std::swprintf(nullptr, 0, format.c_str(),
			_wd_internal::fmt_arg(std::forward<Args>(args))...); // won't include terminating null
		std::wstring buf(nChars + 1, L'\0'); // alloc receiving buffer
#ifdef _MSC_VER
#pragma warning(suppress: 26800, justification: "It's safe to call fmt_arg() twice, because it doesn't move wstring, just calls .c_str().")
#endif
		std::swprintf(buf.data(), buf.size(), format.c_str(), _wd_internal::fmt_arg(std::forward<Args>(args))...);
		buf.resize(nChars);
		return buf;
	}

	[[nodiscard]] inline int compare_i(const std::wstring &s, StrView other) noexcept                               { return StrView{s}.compare_i(other); }
	[[nodiscard]] inline bool eq_i(const std::wstring &s, StrView other) noexcept                                   { return StrView{s}.eq_i(other); }
	[[nodiscard]] std::wstring fmt_bytes(ULONGLONG numBytes);     // Formats the number of bytes contextually.
	[[nodiscard]] inline bool neq_i(const std::wstring &s, StrView other) noexcept                                  { return !eq_i(s, other); }
	[[nodiscard]] std::wstring parse(std::span<const BYTE> blob); // Parses a binary blob into a string.
	[[nodiscard]] inline std::wstring parse(const std::vector<BYTE> &blob)                                          { return parse(std::span{blob}); } // Parses a binary blob into a string.
	std::wstring& remove_diacritics(std::wstring &s) noexcept;
	std::wstring& replace_all(std::wstring &s, StrView what, StrView replacement);
	[[nodiscard]] inline std::vector<std::wstring> split(const std::wstring &s, WCHAR delim)                        { return StrView{s}.split(delim); }
	[[nodiscard]] inline std::vector<std::wstring> split(const std::wstring &s, StrView delim)                      { return StrView{s}.split(delim); }
	[[nodiscard]] inline std::vector<std::wstring> split_lines(const std::wstring &s)                               { return StrView{s}.split_lines(); }
	[[nodiscard]] inline std::vector<std::wstring> split_n(const std::wstring &s, StrView delim, size_t maxSubstrs) { return StrView{s}.split_n(delim, maxSubstrs); }
	[[nodiscard]] inline std::string to_ansi(const std::wstring &s)                                                 { return StrView{s}.to_ansi(); } // Converts a wide string into an ANSI string.
	std::wstring& to_lower(std::wstring &s) noexcept;
	std::wstring& to_upper(std::wstring &s) noexcept;
	[[nodiscard]] inline std::vector<BYTE> to_utf8_blob(const std::wstring &s, bool writeBom = false)               { return StrView{s}.to_utf8_blob(writeBom); }
	[[nodiscard]] std::wstring to_wide(const char *s); // Converts an ANSI string into a wide string.
	std::wstring& trim_left(std::wstring &s, WCHAR charToTrim);
	std::wstring& trim_left(std::wstring &s, StrView charsToTrim);
	std::wstring& trim_right(std::wstring &s, WCHAR charToTrim);
	std::wstring& trim_right(std::wstring &s, StrView charsToTrim);
	std::wstring& trim_spaces(std::wstring &s);

	[[nodiscard]] inline constexpr size_t find(const std::wstring &s, StrView what, size_t off = 0) noexcept                         { return StrView{s}.find(what, off); }
	[[nodiscard]] inline constexpr size_t find_first_not_of(const std::wstring &s, StrView what, size_t off = 0) noexcept            { return StrView{s}.find_first_not_of(what, off); }
	[[nodiscard]] inline constexpr size_t find_first_not_of(const std::wstring &s, WCHAR ch, size_t off = 0) noexcept                { return StrView{s}.find_first_not_of(ch, off); }
	[[nodiscard]] inline constexpr size_t find_first_of(const std::wstring &s, StrView what, size_t off = 0) noexcept                { return StrView{s}.find_first_of(what, off); }
	[[nodiscard]] inline constexpr size_t find_first_of(const std::wstring &s, WCHAR ch, size_t off = 0) noexcept                    { return StrView{s}.find_first_of(ch, off); }
	[[nodiscard]] inline constexpr size_t find_last_not_of(const std::wstring &s, StrView what, size_t off = StrView::npos) noexcept { return StrView{s}.find_last_not_of(what, off); }
	[[nodiscard]] inline constexpr size_t find_last_not_of(const std::wstring &s, WCHAR ch, size_t off = StrView::npos) noexcept     { return StrView{s}.find_last_not_of(ch, off); }
	[[nodiscard]] inline constexpr size_t find_last_of(const std::wstring &s, StrView what, size_t off = StrView::npos) noexcept     { return StrView{s}.find_last_of(what, off); }
	[[nodiscard]] inline constexpr size_t find_last_of(const std::wstring &s, WCHAR ch, size_t off = StrView::npos) noexcept         { return StrView{s}.find_last_of(ch, off); }
	[[nodiscard]] inline constexpr size_t rfind(const std::wstring &s, StrView what, size_t off = StrView::npos) noexcept            { return StrView{s}.rfind(what, off); }

	[[nodiscard]] inline bool starts_with(const std::wstring &s, std::initializer_list<StrView> prefixes) noexcept { return StrView{s}.starts_with(prefixes); }
	[[nodiscard]] inline constexpr bool starts_with(const std::wstring &s, StrView prefix) noexcept                { return StrView{s}.starts_with(prefix); }
	[[nodiscard]] inline constexpr bool starts_with(const std::wstring &s, WCHAR chPrefix) noexcept                { return StrView{s}.starts_with(chPrefix); }
	[[nodiscard]] inline bool ends_with(const std::wstring &s, std::initializer_list<StrView> suffixes) noexcept   { return StrView{s}.ends_with(suffixes); }
	[[nodiscard]] inline constexpr bool ends_with(const std::wstring &s, StrView suffix) noexcept                  { return StrView{s}.ends_with(suffix); }
	[[nodiscard]] inline constexpr bool ends_with(const std::wstring &s, WCHAR chSuffix) noexcept                  { return StrView{s}.ends_with(chSuffix); }
	[[nodiscard]] inline bool starts_with_i(const std::wstring &s, std::initializer_list<StrView> prefixes)        { return StrView{s}.starts_with_i(prefixes); }
	[[nodiscard]] inline bool starts_with_i(const std::wstring &s, StrView prefix)                                 { return StrView{s}.starts_with_i(prefix); }
	[[nodiscard]] inline bool starts_with_i(const std::wstring &s, WCHAR chPrefix) noexcept                        { return StrView{s}.starts_with_i(chPrefix); }
	[[nodiscard]] inline bool ends_with_i(const std::wstring &s, std::initializer_list<StrView> suffixes) noexcept { return StrView{s}.ends_with_i(suffixes); }
	[[nodiscard]] inline bool ends_with_i(const std::wstring &s, StrView suffix) noexcept                          { return StrView{s}.ends_with_i(suffix); }
	[[nodiscard]] inline bool ends_with_i(const std::wstring &s, WCHAR chSuffix) noexcept                          { return StrView{s}.ends_with_i(chSuffix); }

	/// Time formatting options for str::fmt_time().
	enum class Time { ymd, ymd_w, ymd_hm, ymd_hms, ymd_hmsm, hm, hms, hmsm, sm };
	[[nodiscard]] std::wstring fmt_time(const FILETIME &ft, Time format);   ///< Formats time according to options.
	[[nodiscard]] std::wstring fmt_time(const SYSTEMTIME &st, Time format); ///< Formats time according to options.

}

namespace wd {

	/// Calls OutputDebugString() with formatting.
	template<typename ...Args>
	void dbg([[maybe_unused]] wd::StrView format, [[maybe_unused]] Args &&...args) {
	#ifdef _DEBUG
		const std::wstring s = str::fmt(format, std::forward<Args>(args)...);
		OutputDebugStringW(s.c_str());
		OutputDebugStringW(L"\n");
	#endif
	}

}
