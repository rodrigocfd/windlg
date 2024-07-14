#pragma once
#include <string_view>
#include <Windows.h>
#include <CommCtrl.h>

namespace lib {

class ImgList final {
private:
	HIMAGELIST _hImg = nullptr;

public:
	~ImgList() { destroy(); }

	constexpr ImgList() = default;
	ImgList(const ImgList&) = delete;
	ImgList(ImgList&& other) noexcept { operator=(std::forward<ImgList>(other)); }
	ImgList& operator=(const ImgList&) = delete;
	ImgList& operator=(ImgList&& other) noexcept;

	constexpr explicit ImgList(HIMAGELIST hImg) : _hImg{hImg} { }

	const ImgList& addClone(HICON hIcon) const;
	const ImgList& addResource(WORD iconId, HINSTANCE hInst = nullptr) const;
	const ImgList& addShell(std::initializer_list<std::wstring_view> extensions) const;
	ImgList& create(SIZE resolution, UINT ilcFlags = ILC_COLOR32, WORD szInitial = 1, WORD szGrow = 1);
	void destroy() noexcept;
	[[nodiscard]] constexpr HIMAGELIST hImg() const { return _hImg; }
	[[nodiscard]] SIZE resolution() const;
	[[nodiscard]] UINT size() const;
};

}
