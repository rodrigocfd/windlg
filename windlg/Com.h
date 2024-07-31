#pragma once
#include <string_view>
#include <Windows.h>

namespace lib {

// Throws system_error if FAILED(hr).
void checkHr(HRESULT hr, std::string_view funcName = "HRESULT");


// Calls CoInitializeEx() and CoUninitialize().
class Com final {
public:
	~Com();

	Com(const Com&) = delete;
	Com(Com&&) = delete;
	Com& operator=(const Com&) = delete;
	Com& operator=(Com&&) = delete;

	explicit Com(DWORD coInit = COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
};


// Calls OleInitialize() and OleUninitialize().
class ComOle final {
public:
	~ComOle();

	ComOle(const ComOle&) = delete;
	ComOle(ComOle&&) = delete;
	ComOle& operator=(const ComOle&) = delete;
	ComOle& operator=(ComOle&&) = delete;

	ComOle();
};


// Templated COM smart pointer.
template<typename T,
	typename = std::enable_if_t<std::is_base_of_v<IUnknown, T>>>
class ComPtr final {
public:
	~ComPtr() { release(); }

	constexpr ComPtr() = default;
	ComPtr(const ComPtr& other) { operator=(other); }
	ComPtr(ComPtr&& other) noexcept { operator=(std::forward<ComPtr<T>>(other)); }

	ComPtr& operator=(const ComPtr& other) {
		release();
		if (other._p) {
			other._p->AddRef(); // we're effectively cloning the COM pointer
			_p = other._p;
		}
		return *this;
	}

	ComPtr& operator=(ComPtr&& other) noexcept {
		release();
		std::swap(_p, other._p);
		return *this;
	}

	constexpr explicit ComPtr(T* p) : _p{p} { }
	explicit ComPtr(REFCLSID clsid, DWORD clsctx = CLSCTX_INPROC_SERVER) { coCreateInstance(clsid, clsctx); }

	[[nodiscard]] constexpr T* operator->() const { return _p; }
	[[nodiscard]] constexpr T* ptr() const { return _p; }
	[[nodiscard]] constexpr T** pptr() { return &_p; }

	// Creates the COM pointer with CoCreateInstance().
	void coCreateInstance(REFCLSID clsid, DWORD clsctx = CLSCTX_INPROC_SERVER) {
		release();
		checkHr(CoCreateInstance(clsid, nullptr, clsctx, IID_PPV_ARGS(&_p)), "CoCreateInstance");
	}

	// Creates the COM pointer with QueryInterface().
	template<typename Q,
		typename = std::enable_if_t<std::is_base_of_v<IUnknown, Q>>>
	[[nodiscard]] ComPtr<Q> queryInterface() const {
		Q* pQueried = nullptr;
		checkHr(_p->QueryInterface(IID_PPV_ARGS(&pQueried)), "QueryInterface");
		return ComPtr<Q>{pQueried};
	}

	// Manually calls Release(). The destructor will call this method automatically.
	void release() noexcept {
		if (_p) {
			_p->Release();
			_p = nullptr;
		}
	}

private:
	T* _p = nullptr;
};

}
