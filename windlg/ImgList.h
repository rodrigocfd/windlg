#pragma once
#include <string_view>
#include <Windows.h>
#include <CommCtrl.h>

namespace lib {

// Image list used with common controls.
class ImgList final {
public:
	~ImgList() { destroy(); }

	constexpr ImgList() = default;
	ImgList(const ImgList&) = delete;
	ImgList(ImgList&& other) noexcept { operator=(std::forward<ImgList>(other)); }
	ImgList& operator=(const ImgList&) = delete;
	ImgList& operator=(ImgList&& other) noexcept;

	constexpr explicit ImgList(HIMAGELIST hImg) : _hImg{hImg} { }

	// Stores a copy of the HICON.
	const ImgList& addCopy(HICON hIcon) const;

	// Stores a HICON loaded with LoadImage().
	const ImgList& addResource(WORD iconId, HINSTANCE hInst = nullptr) const;

	// Stores the HICONs associated to the given file extensions, loaded with SHGetFileInfo().
	const ImgList& addShell(std::initializer_list<std::wstring_view> extensions) const;

	[[nodiscard]] UINT count() const;
	ImgList& create(SIZE resolution, UINT ilcFlags = ILC_COLOR32, WORD szInitial = 1, WORD szGrow = 1);

	// The destructor will call this method automatically.
	void destroy() noexcept;

	[[nodiscard]] constexpr HIMAGELIST hImg() const { return _hImg; }
	[[nodiscard]] SIZE resolution() const;

private:
	HIMAGELIST _hImg = nullptr;
};

}
