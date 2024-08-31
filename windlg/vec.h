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
template<typename T>
void append(std::vector<T>& dest, const std::type_identity_t<T>& elem) {
	dest.push_back(elem);
}
// Appends multiple elements to the vector with push_back().
template<typename T, typename... U>
void append(std::vector<T>& dest, const std::type_identity_t<T>& elem, U... rest) {
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

// Returns a pointer to the first found element, or nullptr.
template<std::ranges::contiguous_range R,
	typename T = std::remove_reference_t<std::ranges::range_reference_t<R>> >
	requires std::ranges::sized_range<R>
[[nodiscard]] T* find(R&& v, const std::type_identity_t<T>& elem) {
	auto foundIt = std::find(v.begin(), v.end(), elem);
	return (foundIt == v.end()) ? nullptr : &(*foundIt);
}
// Returns a pointer to the first element according to the predicate, or nullptr.
// Example:
// findIf(entries, [](const Entry&) -> bool { return true; });
template<std::ranges::contiguous_range R,
	typename T = std::remove_reference_t<std::ranges::range_reference_t<R>> >
	requires std::ranges::sized_range<R>
[[nodiscard]] T* findIf(R&& v, std::predicate<T> auto pred) {
	auto foundIt = std::find_if(v.begin(), v.end(), pred);
	return (foundIt == v.end()) ? nullptr : &(*foundIt);
}
// Returns a pointer to the last found element, or nullptr.
template<std::ranges::contiguous_range R,
	typename T = std::remove_reference_t<std::ranges::range_reference_t<R>> >
	requires std::ranges::sized_range<R>
[[nodiscard]] T* findRev(R&& v, const std::type_identity_t<T>& elem) {
	auto foundIt = std::find(v.rbegin(), v.rend(), elem);
	return (foundIt == v.rend()) ? nullptr : &(*foundIt);
}
// Returns a pointer to the last element according to the predicate, or nullptr.
// Example:
// findRefIf(entries, [](const Entry&) -> bool { return true; });
template<std::ranges::contiguous_range R,
	typename T = std::remove_reference_t<std::ranges::range_reference_t<R>> >
	requires std::ranges::sized_range<R>
[[nodiscard]] T* findRevIf(R&& v, std::predicate<T> auto pred) {
	auto foundIt = std::find_if(v.rbegin(), v.rend(), pred);
	return (foundIt == v.rend()) ? nullptr : &(*foundIt);
}

// Creates a vector and calls reserve().
template<typename T>
[[nodiscard]] std::vector<T> newReserved(size_t numReserve) {
	std::vector<T> v;
	v.reserve(numReserve);
	return v;
}

// Returns the index of the first found element.
template<std::ranges::contiguous_range R,
	typename T = std::remove_reference_t<std::ranges::range_reference_t<R>> >
	requires std::ranges::sized_range<R>
[[nodiscard]] std::optional<size_t> position(R&& v, const std::type_identity_t<T>& elem) {
	auto foundIt = std::find(v.begin(), v.end(), elem);
	return (foundIt == v.end()) ? std::nullopt : std::optional{std::distance(v.begin(), foundIt)};
}
// Returns the index of the last found element.
template<std::ranges::contiguous_range R,
	typename T = std::remove_reference_t<std::ranges::range_reference_t<R>> >
	requires std::ranges::sized_range<R>
[[nodiscard]] std::optional<size_t> positionRev(R&& v, const std::type_identity_t<T>& elem) {
	auto foundIt = std::find(v.rbegin(), v.rend(), elem);
	return (foundIt == v.rend()) ? std::nullopt : std::optional{std::distance(foundIt, std::prev(v.rend()))};
}
// Returns the first index according to the predicate.
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

// Returns the index of the first element where the entire sequence matches the following elements.
template<std::ranges::contiguous_range R,
	typename T = std::remove_reference_t<std::ranges::range_reference_t<R>> >
	requires std::ranges::sized_range<R>
[[nodiscard]] std::optional<size_t> positionSeq(R&& v,
		std::initializer_list<const std::type_identity_t<T>> sequence) {
	for (size_t i = 0; i <= v.size() - sequence.size(); ++i) {
		bool found = true;
		for (size_t e = 0; e < sequence.size(); ++e) {
			if (v[e + i] != *(sequence.begin() + e)) {
				found = false;
				break;
			}
		}
		if (found)
			return {i};
	}
	return std::nullopt;
}

// Removes the element at the given index.
template<typename T>
void remove(std::vector<T>& v, size_t index) {
	v.erase(v.begin() + index);
}
// Removes the elements to which the callback returns true.
// Example:
// removeIf(entries, [](const Entry&) -> bool { return true; });
template<typename T>
void removeIf(std::vector<T>& v, std::predicate<T> auto pred) {
	v.erase(std::remove_if(v.begin(), v.end(), pred), v.end());
}

// Returns spans over the source vector, splitted by the delimiter, including empty spans.
template<std::ranges::contiguous_range R,
	typename T = std::remove_reference_t<std::ranges::range_reference_t<R>> >
	requires std::ranges::sized_range<R>
[[nodiscard]] std::vector<std::span<T>> split(
		R&& v, const std::type_identity_t<T>& delimiter, std::optional<UINT> maxParts = std::nullopt) {
	if (v.empty()) return {};

	std::span<T> src{v};
	size_t count = 0, head = 0;
	for (;;) {
		if (maxParts.has_value() && maxParts.value() && count >= maxParts.value() - 1)
			break;

		std::optional<size_t> maybeHead = position(src, delimiter);
		if (!maybeHead.has_value()) {
			break;
		} else {
			head = maybeHead.value();
			++count;
			++head;
			src = src.subspan(head);
		}
	}
	++count;

	std::vector<std::span<T>> ret;
	ret.reserve(count);
	
	src = v;
	head = 0;
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

// Returns a new vector by applying the callback to each element.
// Example:
// vector<int> v = transform(entries, [](const Entry&) -> int { return 9; });
template<std::ranges::contiguous_range R,
	typename T = std::remove_reference_t<std::ranges::range_reference_t<R>>,
	typename F = std::is_invocable<const std::type_identity_t<T>&>,
	typename U = std::invoke_result_t<F, const std::type_identity_t<T>&> >
	requires std::ranges::sized_range<R>
[[nodiscard]] std::vector<U> transform(R&& v, F callback) {
	std::vector<U> ret;
	ret.reserve(v.size());
	for (auto&& elem : v)
		ret.emplace_back(callback(elem));
	return ret;
}

}
