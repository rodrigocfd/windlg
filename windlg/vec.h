#pragma once
#include <optional>
#include <ranges>
#include <span>
#include <vector>

namespace lib::vec {

// Appends a single element to the vector. Same as push_back().
template<typename T>
void append(std::vector<T>& dest, const T& elem) {
	dest.push_back(elem);
}
// Appends multiple elements to the vector.
template<typename T, typename... U>
void append(std::vector<T>& dest, const T& elem, const U&... elems) {
	append(dest, elem);
	append(dest, elems...);
}
// Appends all elements of vectors to the vector.
template<std::ranges::contiguous_range R,
	typename T = std::ranges::range_value_t<R>>
void append(std::vector<T>& dest, R&& foreigner) {
	dest.insert(dest.end(), foreigner.begin(), foreigner.end());
}
// Appends all elements of all vectors to the vector.
template<std::ranges::contiguous_range R,
	typename T = std::ranges::range_value_t<R>,
	typename... U>
void append(std::vector<T>& dest, R&& foreigner, U... foreigners) {
	append(dest, foreigner);
	append(dest, foreigners...);
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
	typename T = std::ranges::range_value_t<R>>
[[nodiscard]] std::optional<size_t> position(R&& v, const T& elem) {
	auto foundIt = std::find(v.begin(), v.end(), elem);
	return (foundIt == v.end()) ? std::nullopt : std::optional{std::distance(v.begin(), foundIt)};
}
// Returns the index of the last element.
template<std::ranges::contiguous_range R,
	typename T = std::ranges::range_value_t<R>>
[[nodiscard]] std::optional<size_t> positionRev(R&& v, const T& elem) {
	auto foundIt = std::find(v.rbegin(), v.rend(), elem);
	return (foundIt == v.rend()) ? std::nullopt : std::optional{std::distance(foundIt, std::prev(v.rend()))};
}
// Returns the index according to the predicate.
// Example:
// position_if(entries, [](const Entry&) -> bool { return true; });
template<std::ranges::contiguous_range R,
	typename T = std::ranges::range_value_t<R>>
[[nodiscard]] std::optional<size_t> positionIf(R&& v, std::predicate<T> auto pred) {
	auto foundIt = std::find_if(v.begin(), v.end(), pred);
	return (foundIt == v.end()) ? std::nullopt : std::optional{std::distance(v.begin(), foundIt)};
}
// Returns the last index according to the predicate.
// Example:
// position_if(entries, [](const Entry&) -> bool { return true; });
template<std::ranges::contiguous_range R,
	typename T = std::ranges::range_value_t<R>>
[[nodiscard]] std::optional<size_t> positionRevIf(R&& v, std::predicate<T> auto pred) {
	auto foundIt = std::find_if(v.rbegin(), v.rend(), pred);
	return (foundIt == v.rend()) ? std::nullopt : std::optional{std::distance(foundIt, std::prev(v.rend()))};
}

// Returns spans over the source vector, splitted by the delimiter, including empty spans.
template<std::ranges::contiguous_range R,
	typename T = std::ranges::range_value_t<R>>
[[nodiscard]] std::vector<std::span<T>> split(R&& v, const T& delimiter) {
	if (v.empty()) return {};

	std::span<T> src{v.begin(), v.end()};
	std::vector<std::span<T>> ret;
	size_t head = 0;
	for (;;) {
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
