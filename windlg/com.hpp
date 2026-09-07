#pragma once
#include <type_traits>
#include "win-error.hpp"
#include <combaseapi.h>

namespace wd {

	/// Templated COM smart pointer.
	template<typename T>
		requires std::is_base_of_v<IUnknown, T>
	class ComPtr final {
	public:
		~ComPtr() noexcept { release(); }
		ComPtr() = default;
		constexpr explicit ComPtr(T *p) noexcept : _p{p} { }

		ComPtr(const ComPtr &other) noexcept { operator=(other); }
		ComPtr& operator=(const ComPtr &other) noexcept {
			release();
			if (other._p) {
				other._p->AddRef(); // we're effectively cloning the COM pointer
				_p = other._p;
			}
			return *this;
		}

		constexpr ComPtr(ComPtr &&other) noexcept : _p{other._p} { other._p = nullptr; }
		ComPtr& operator=(ComPtr &&other) noexcept {
			release();
			std::swap(_p, other._p);
			return *this;
		}

		ComPtr& operator=(T *p) noexcept {
			release();
			_p = p; // take ownership
			return *this;
		}

		[[nodiscard]] constexpr T* operator->() const noexcept { return _p; }
		[[nodiscard]] constexpr T* ptr() const noexcept        { return _p; }

		template<typename Q = T>
		[[nodiscard]] constexpr Q** pptr() noexcept { return reinterpret_cast<Q**>(&_p); }

		[[nodiscard]] T* leak() noexcept {
			T *ptr = _p;
			_p = nullptr;
			return ptr;
		}

		void co_create_instance(REFCLSID clsid, DWORD clsctx = CLSCTX_INPROC_SERVER) {
			release();
			if (HRESULT hr = CoCreateInstance(clsid, nullptr, clsctx, IID_PPV_ARGS(&_p)); FAILED(hr)) [[unlikely]] {
				throw WinErr{hr, L"CoCreateInstance failed."};
			}
		}

		template<typename Q>
			requires std::is_base_of_v<IUnknown, Q>
		[[nodiscard]] ComPtr<Q> query_interface() const {
			Q *pQueried = nullptr;
			if (HRESULT hr = _p->QueryInterface(IID_PPV_ARGS(&pQueried)); FAILED(hr)) [[unlikely]] {
				throw WinErr{hr, L"QueryInterface failed."};
			}
			return ComPtr<Q>{pQueried};
		}

		void release() noexcept {
			if (_p) {
				_p->Release();
				_p = nullptr;
			}
		}

	private:
		T *_p = nullptr;
	};

}
