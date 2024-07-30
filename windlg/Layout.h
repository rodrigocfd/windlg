#pragma once
#include <optional>
#include <vector>
#include "Dialog.h"

namespace lib {

// Repositions and resizes child controls when the parent window moves and resizes.
class Layout final {
public:
	enum class Act { None, Repos, Resize };

	constexpr Layout() = default;
	Layout(const Layout&) = delete;
	Layout(Layout&&) = delete;
	Layout& operator=(const Layout&) = delete;
	Layout& operator=(Layout&&) = delete;

	Layout& add(Act horz, Act vert, std::initializer_list<WORD> ctrlIds);
	void autoArrange(Dialog* parent, UINT uMsg, WPARAM wp, LPARAM lp);

private:
	struct ChildInfo final {
		WORD ctrlId = 0;
		Act horz = Act::None;
		Act vert = Act::None;
		std::optional<RECT> rcOrig{std::nullopt};
	};

	void _rearrangeCtrls(UINT cxParent, UINT cyParent);

	HWND _hParent = nullptr;
	std::vector<ChildInfo> _ctrls;
	SIZE _szParentOrig{};
};

}
