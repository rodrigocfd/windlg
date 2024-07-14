#include <system_error>
#include <Windows.h>
#include <CommCtrl.h>
#include <commoncontrols.h> // IImageList
#include "Com.h"
#include "ImgList.h"
using namespace lib;

ImgList& ImgList::operator=(ImgList&& other) noexcept
{
	destroy();
	std::swap(_hImg, other._hImg);
	return *this;
}

const ImgList& ImgList::addClone(HICON hIcon) const
{
	ImageList_AddIcon(_hImg, hIcon);
	return *this;
}

const ImgList& ImgList::addResource(WORD iconId, HINSTANCE hInst) const
{
	SIZE res = resolution();

	HICON hIcon = static_cast<HICON>(LoadImageW(hInst ? hInst : GetModuleHandleW(nullptr),
		MAKEINTRESOURCEW(iconId), IMAGE_ICON,
		static_cast<int>(res.cx), static_cast<int>(res.cy),
		LR_DEFAULTCOLOR));
	if (!hIcon) [[unlikely]] {
		throw std::system_error(GetLastError(), std::system_category(), "LoadImage failed");
	}
	addClone(hIcon);
	DestroyIcon(hIcon);
	return *this;
}

const ImgList& ImgList::addShell(std::initializer_list<std::wstring_view> extensions) const
{
	SIZE res = resolution();

	for (auto&& extension : extensions) {
		WCHAR ext[16] = {L'\0'};
		lstrcpyW(ext, L"*."); // prepend
		lstrcatW(ext, extension.data());

	
		SHFILEINFOW shfi{};

		if ((res.cx == 16 && res.cy == 16) || (res.cx == 32 && res.cy == 32)) { // http://stackoverflow.com/a/28015423
			DWORD_PTR gfiOk = SHGetFileInfoW(ext, FILE_ATTRIBUTE_NORMAL, &shfi, sizeof(shfi),
				SHGFI_USEFILEATTRIBUTES | SHGFI_ICON |
				(res.cx == 16 ? SHGFI_SMALLICON : SHGFI_LARGEICON));
			if (!gfiOk) [[unlikely]] {
				throw std::system_error(GetLastError(), std::system_category(), "SHGetFileInfo failed");
			}
			addClone(shfi.hIcon);
			if (shfi.hIcon) DestroyIcon(shfi.hIcon);
		} else { // http://stackoverflow.com/a/30496252
			int shil = 0;
			switch (res.cx) {
				case 32:  shil = SHIL_LARGE; break;
				case 16:  shil = SHIL_SMALL; break;
				case 48:  shil = SHIL_EXTRALARGE; break;
				case 256: shil = SHIL_JUMBO; break;
				default:  throw new std::logic_error("Invalid image list icon size");
			}

			ComPtr<IImageList> comImg;
			if (HRESULT hr = SHGetImageList(shil, IID_IImageList,
					reinterpret_cast<void**>(comImg.pptr())); FAILED(hr)) [[unlikely]] {
				throw std::system_error(hr, std::system_category(), "SHGetImageList failed");
			}

			DWORD_PTR gfiOk = SHGetFileInfoW(ext, FILE_ATTRIBUTE_NORMAL, &shfi, sizeof(shfi),
				SHGFI_USEFILEATTRIBUTES | SHGFI_SYSICONINDEX);
			if (!gfiOk) [[unlikely]] {
				throw std::system_error(GetLastError(), std::system_category(), "SHGetFileInfo failed");
			}
			addClone(shfi.hIcon);
			if (shfi.hIcon) DestroyIcon(shfi.hIcon);
		}
	}
	return *this;
}

ImgList& ImgList::create(SIZE resolution, UINT ilcFlags, WORD szInitial, WORD szGrow)
{
	destroy();
	_hImg = ImageList_Create(resolution.cx, resolution.cy, ilcFlags,
		static_cast<int>(szInitial), static_cast<int>(szGrow));
	if (!_hImg) [[unlikely]] {
		throw std::system_error(GetLastError(), std::system_category(), "ImageList_Create failed");
	}
	return *this;
}

void ImgList::destroy() noexcept
{
	if (_hImg) {
		ImageList_Destroy(_hImg);
		_hImg = nullptr;
	}
}

SIZE ImgList::resolution() const
{
	SIZE res{};
	ImageList_GetIconSize(_hImg, reinterpret_cast<int*>(&res.cx), reinterpret_cast<int*>(&res.cy));
	return res;
}

UINT ImgList::size() const
{
	return ImageList_GetImageCount(_hImg);
}
