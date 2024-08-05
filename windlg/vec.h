#pragma once
#include <optional>
#include <ranges>
#include <span>
#include <vector>

namespace lib::vec {

// Returns true if all elements are equal to the given one.
template<std::ranges::contiguous_range R,
	typename T = std::remove_reference_t<std::ranges::range_reference_t<R>> >
	requires std::ranges::sized_range<R>
[[nodiscard]] bool all(R&& v, const std::type_identity_t<T>& elem) {
	for (auto it = v.begin(); it != v.end(); ++it) {
		if (*it != elem) return false;
	}
	return true;
}
// Returns true if the predicate returns true for all of the elements.
// Example:
// allIf(entries, [](const Entry&) -> bool { return true; });
template<std::ranges::contiguous_range R,
	typename T = std::remove_reference_t<std::ranges::range_reference_t<R>> >
	requires std::ranges::sized_range<R>
[[nodiscard]] bool allIf(R&& v, std::predicate<T> auto pred) {
	for (auto it = v.begin(); it != v.end(); ++it) {
		if (!pred(*it)) return false;
	}
	return true;
}

// Returns true if one of the elements is equal to the given one.
template<std::ranges::contiguous_range R,
	typename T = std::remove_reference_t<std::ranges::range_reference_t<R>> >
	requires std::ranges::sized_range<R>
[[nodiscard]] bool any(R&& v, const std::type_identity_t<T>& elem) {
	return std::find(v.begin(), v.end(), elem) != v.end();
}
// Returns true if the predicate returns true for any of the elements.
// Example:
// anyIf(entries, [](const Entry&) -> bool { return true; });
template<std::ranges::contiguous_range R,
	typename T = std::remove_reference_t<std::ranges::range_reference_t<R>> >
	requires std::ranges::sized_range<R>
[[nodiscard]] bool anyIf(R&& v, std::predicate<T> auto pred) {
	return std::find_if(v.begin(), v.end(), pred) != v.end();
}

// Appends multiple elements to the vector with push_back().
template<typename T,
	typename E = std::vector<T>::value_type>
void append(std::vector<T>& dest, const E& elem) {
	dest.push_back(elem);
}
// Appends multiple elements to the vector with push_back().
template<typename T,
	typename E = std::vector<T>::value_type,
	typename... U>
void append(std::vector<T>& dest, const E& elem, U... rest) {
	append(dest, elem);
	append(dest, rest...);
}

// Appends all elements of vectors to the vector with insert().
template<std::ranges::contiguous_range R,
	typename T = std::remove_reference_t<std::ranges::range_reference_t<R>> > // https://stackoverflow.com/q/78827063
	requires std::ranges::sized_range<R>
void append(std::vector<T>& dest, R&& other) {
	dest.insert(dest.end(), other.begin(), other.end());
}
// Appends all elements of vectors to the vector with insert().
template<std::ranges::contiguous_range R,
	typename T = std::remove_reference_t<std::ranges::range_reference_t<R>>,
	typename... U>
	requires std::ranges::sized_range<R>
void append(std::vector<T>& dest, R&& other, U... rest) {
	append(dest, std::forward<R>(other));
	append(dest, rest...);
}

// Creates a vector and calls reserve().
template<typename T>
[[nodiscard]] std::vector<T> newReserved(size_t numReserve) {
	std::vector<T> v;
	v.reserve(numReserve);
	return v;
}

// Returns the index of the first element.
template<std::ranges::contiguous_range R,
	typename T = std::remove_reference_t<std::ranges::range_reference_t<R>> >
	requires std::ranges::sized_range<R>
[[nodiscard]] std::optional<size_t> position(R&& v, const std::type_identity_t<T>& elem) {
	auto foundIt = std::find(v.begin(), v.end(), elem);
	return (foundIt == v.end()) ? std::nullopt : std::optional{std::distance(v.begin(), foundIt)};
}
// Returns the index of the last element.
template<std::ranges::contiguous_range R,
	typename T = std::remove_reference_t<std::ranges::range_reference_t<R>> >
	requires std::ranges::sized_range<R>
[[nodiscard]] std::optional<size_t> positionRev(R&& v, const std::type_identity_t<T>& elem) {
	auto foundIt = std::find(v.rbegin(), v.rend(), elem);
	return (foundIt == v.rend()) ? std::nullopt : std::optional{std::distance(foundIt, std::prev(v.rend()))};
}
// Returns the index according to the predicate.
// Example:
// positionIf(entries, [](const Entry&) -> bool { return true; });
template<std::ranges::contiguous_range R,
	typename T = std::remove_reference_t<std::ranges::range_reference_t<R>> >
	requires std::ranges::sized_range<R>
[[nodiscard]] std::optional<size_t> positionIf(R&& v, std::predicate<T> auto pred) {
	auto foundIt = std::find_if(v.begin(), v.end(), pred);
	return (foundIt == v.end()) ? std::nullopt : std::optional{std::distance(v.begin(), foundIt)};
}
// Returns the last index according to the predicate.
// Example:
// positionRefIf(entries, [](const Entry&) -> bool { return true; });
template<std::ranges::contiguous_range R,
	typename T = std::remove_reference_t<std::ranges::range_reference_t<R>> >
	requires std::ranges::sized_range<R>
[[nodiscard]] std::optional<size_t> positionRevIf(R&& v, std::predicate<T> auto pred) {
	auto foundIt = std::find_if(v.rbegin(), v.rend(), pred);
	return (foundIt == v.rend()) ? std::nullopt : std::optional{std::distance(foundIt, std::prev(v.rend()))};
}

// Returns spans over the source vector, splitted by the delimiter, including empty spans.
template<std::ranges::contiguous_range R,
	typename T = std::remove_reference_t<std::ranges::range_reference_t<R>> >
	requires std::ranges::sized_range<R>
[[nodiscard]] std::vector<std::span<T>> split(
		R&& v, const std::type_identity_t<T>& delimiter, std::optional<UINT> maxParts = std::nullopt) {
	if (v.empty()) return {};

	std::span<T> src{v};
	std::vector<std::span<T>> ret;
	size_t head = 0;
	for (;;) {
		if (maxParts.has_value() && maxParts.value() && ret.size() >= maxParts.value() - 1)
			break;

		std::optional<size_t> maybeHead = position(src, delimiter);
		if (!maybeHead.has_value()) {
			break;
		} else {
			head = maybeHead.value();
			ret.emplace_back(src.subspan(0, head));
			++head;
			src = src.subspan(head);
		}
	}

	ret.emplace_back(src);
	return ret;
}

}
