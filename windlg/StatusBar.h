#pragma once
#include <string>
#include <vector>
#include "Dialog.h"
#include "NativeControl.h"

namespace lib {

// Status bar native control.
class StatusBar final : public NativeControl {
public:
	class Part final {
	public:
		Part() = delete;
		constexpr Part(const Part&) = default;
		constexpr Part(Part&&) = default;
		constexpr Part& operator=(const Part&) = default;
		constexpr Part& operator=(Part&&) = default;

		constexpr Part(HWND hSb, int index) : _hSb{hSb}, _index{index} { }
		constexpr Part(const StatusBar* owner, int index) : Part{owner->hWnd(), index} { }

		[[nodiscard]] constexpr int index() const { return _index; }
		const Part& setIcon(HICON hIcon) const;
		const Part& setText(std::wstring_view text) const;
		[[nodiscard]] std::wstring text() const;

	private:
		HWND _hSb = nullptr;
		int _index = -1;
	};

private:
	class PartCollection final {
		friend StatusBar;
	private:
		constexpr explicit PartCollection(const StatusBar* pSb) : _pSb{pSb} { }

	public:
		PartCollection() = delete;
		PartCollection(const PartCollection&) = delete;
		PartCollection(PartCollection&&) = delete;
		PartCollection& operator=(const PartCollection&) = delete;
		PartCollection& operator=(PartCollection&&) = delete;

		[[nodiscard]] constexpr Part operator[](int index) const { return Part{_pSb, index}; }
		Part addFixed(UINT width, std::wstring_view text = L"") { return _addPart(PartData{width, 0}, text); }
		Part addResizable(BYTE resizeWeight, std::wstring_view text = L"") { return _addPart(PartData{0, resizeWeight}, text); }
		[[nodiscard]] constexpr UINT count() const { return static_cast<UINT>(_partsData.size()); }

	private:
		struct PartData final {
			UINT sizePixels = 0; // mutually exclusive
			BYTE resizeWeight = 0;
		};

		Part _addPart(PartData partData, std::wstring_view text);
		UINT _parentWidth() const;
		void _resizeToParent(UINT cxParent);

		const StatusBar* _pSb = nullptr; // assumes StatusBar is immovable
		std::vector<PartData> _partsData;
		std::vector<int> _rightEdges;
	};

public:
	PartCollection parts{this};

	virtual ~StatusBar() { }

	constexpr StatusBar() = default;
	StatusBar(const StatusBar&) = delete;
	StatusBar(StatusBar&&) = delete;
	StatusBar& operator=(const StatusBar&) = delete;
	StatusBar& operator=(StatusBar&&) = delete;

	// Creates a new StatusBar by calling CreateWindowEx().
	const StatusBar& create(Dialog* parent, WORD ctrlId = 0);

	// To be called during parent's WM_SIZE processing.
	const StatusBar& resizeToParent(UINT cxParent);

private:
	Window::_hWndPtr;
};

}
