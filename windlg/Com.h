#pragma once
#include <string_view>
#include <Windows.h>

namespace lib {

// Loads and unloads the COM stuff.
class Com final {
private:
	HRESULT _hr = S_OK;

public:
	~Com();

	Com(const Com&) = delete;
	Com(Com&&) = delete;
	Com& operator=(const Com&) = delete;
	Com& operator=(Com&&) = delete;

	explicit Com(DWORD coInit = COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	// Value returned by the CoInitializeEx() call.
	[[nodiscard]] constexpr HRESULT result() const { return _hr; }

	// Throws system_error if FAILED(hr).
	static void Check(HRESULT hr, std::string_view funcName = "HRESULT");
};


// Templated COM smart pointer.
template<typename T,
	typename = std::enable_if_t<std::is_base_of_v<IUnknown, T>>>
class ComPtr final {
private:
	T* _p = nullptr;

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
		Com::Check(CoCreateInstance(clsid, nullptr, clsctx, IID_PPV_ARGS(&_p)), "CoCreateInstance");
	}

	// Creates the COM pointer with QueryInterface().
	template<typename Q,
		typename = std::enable_if_t<std::is_base_of_v<IUnknown, Q>>>
	[[nodiscard]] ComPtr<Q> queryInterface() const {
		Q* pQueried = nullptr;
		Com::Check(_p->QueryInterface(IID_PPV_ARGS(&pQueried)), "QueryInterface");
		return ComPtr<Q>{pQueried};
	}

	// Manually calls Release(). The destructor will call this method automatically.
	void release() noexcept {
		if (_p) {
			_p->Release();
			_p = nullptr;
		}
	}
};

}
