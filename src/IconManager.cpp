#include "IconManager.h"
#include <shellapi.h>
#include <objbase.h>
#include <vector> // For std::vector used in ConvertHICONToD2DBitmap (though not directly, good include)

// Ensure this is defined or included from a common header
extern ns::ApplicationSettings appSettings;
extern PersistenceManager* pPersistenceManager; // If IconManager needs to interact with it (currently doesn't directly)


template<class Interface>
inline void SafeRelease(Interface **ppInterfaceToRelease) {
    if (*ppInterfaceToRelease != nullptr) {
        (*ppInterfaceToRelease)->Release();
        (*ppInterfaceToRelease) = nullptr;
    }
}

IconManager::IconManager() : m_pRenderTarget(nullptr), m_pWICFactory(nullptr) {
}

IconManager::~IconManager() {
    Shutdown();
}

HRESULT IconManager::Initialize(ID2D1RenderTarget* pRT) {
    m_pRenderTarget = pRT;

    if (!m_pWICFactory) {
        HRESULT hr = CoCreateInstance(
            CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&m_pWICFactory)
        );
        if (FAILED(hr)) return hr;
    }
    return S_OK;
}

void IconManager::Shutdown() {
    ReleaseDeviceResources();
    SafeRelease(&m_pWICFactory);
    m_pRenderTarget = nullptr;
}

void IconManager::ReleaseDeviceResources() {
    for (auto& pair : m_iconBitmapCache) {
        SafeRelease(&(pair.second));
    }
    m_iconBitmapCache.clear();

    for (auto& pair : m_genericImageCache) {
        SafeRelease(&(pair.second));
    }
    m_genericImageCache.clear();

    // Nullify pBitmap in all DesktopIcon instances across all pages and zones
    // This is crucial because these bitmaps are device-dependent.
    // No need to check pPersistenceManager here, as appSettings is global.
    for (auto& page : appSettings.pages) {
        for (auto& zone : page.zones) {
            for (auto& icon : zone.icons) {
                icon.pBitmap = nullptr; // The actual SafeRelease happened above if it was in cache
                                        // Or it was never cached / already released.
                                        // This ensures stale pointers are cleared.
            }
        }
    }
}


ID2D1Bitmap* IconManager::GetIconBitmap(ns::DesktopIcon& iconInfo) {
    if (!m_pRenderTarget || !m_pWICFactory) return nullptr;

    // If pBitmap is already set and valid for the current render target context, return it.
    // For simplicity, we assume if non-null, it's from the current RT context.
    // ReleaseDeviceResources() should have cleared it if RT changed.
    if (iconInfo.pBitmap) return iconInfo.pBitmap;

    auto it = m_iconBitmapCache.find(iconInfo.path);
    if (it != m_iconBitmapCache.end() && it->second != nullptr) {
        iconInfo.pBitmap = it->second;
        return it->second; // Return cached bitmap
    }

    HICON hIcon = nullptr;
    SHFILEINFOW sfi = {0};
    UINT flags = SHGFI_ICON | SHGFI_LARGEICON;

    std::wstring pathToQuery = iconInfo.path;

    DWORD_PTR result = SHGetFileInfoW(pathToQuery.c_str(), 0, &sfi, sizeof(sfi), flags);

    if (result != 0 && sfi.hIcon != NULL) {
        hIcon = sfi.hIcon;
    } else {
        flags = SHGFI_ICON | SHGFI_SMALLICON;
        result = SHGetFileInfoW(pathToQuery.c_str(), 0, &sfi, sizeof(sfi), flags);
        if (result != 0 && sfi.hIcon != NULL) {
            hIcon = sfi.hIcon;
        } else {
            return nullptr;
        }
    }

    ID2D1Bitmap* pD2DBitmap = nullptr;
    if (hIcon) {
        HRESULT hr = ConvertHICONToD2DBitmap(hIcon, &pD2DBitmap);
        DestroyIcon(hIcon);

        if (SUCCEEDED(hr) && pD2DBitmap) {
            m_iconBitmapCache[iconInfo.path] = pD2DBitmap;
            iconInfo.pBitmap = pD2DBitmap;
            return pD2DBitmap;
        } else {
            SafeRelease(&pD2DBitmap);
        }
    }
    return nullptr;
}

ID2D1Bitmap* IconManager::LoadImageFileAsBitmap(const std::wstring& imagePath) {
    if (imagePath.empty() || !m_pRenderTarget || !m_pWICFactory) {
        return nullptr;
    }

    auto it = m_genericImageCache.find(imagePath);
    if (it != m_genericImageCache.end() && it->second != nullptr) {
        return it->second;
    }

    IWICBitmapDecoder *pDecoder = nullptr;
    IWICBitmapFrameDecode *pSource = nullptr;
    IWICFormatConverter *pConverter = nullptr;
    ID2D1Bitmap *pD2DBitmap = nullptr;

    HRESULT hr = m_pWICFactory->CreateDecoderFromFilename(
        imagePath.c_str(), NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pDecoder);

    if (SUCCEEDED(hr)) hr = pDecoder->GetFrame(0, &pSource);
    if (SUCCEEDED(hr)) hr = m_pWICFactory->CreateFormatConverter(&pConverter);
    if (SUCCEEDED(hr)) {
        hr = pConverter->Initialize(
            pSource, GUID_WICPixelFormat32bppPBGRA,
            WICBitmapDitherTypeNone, NULL, 0.f, WICBitmapPaletteTypeMedianCut
        );
    }
    if (SUCCEEDED(hr)) {
        hr = m_pRenderTarget->CreateBitmapFromWicBitmap(pConverter, NULL, &pD2DBitmap);
    }

    SafeRelease(&pDecoder);
    SafeRelease(&pSource);
    SafeRelease(&pConverter);

    if (SUCCEEDED(hr) && pD2DBitmap) {
        m_genericImageCache[imagePath] = pD2DBitmap;
        return pD2DBitmap;
    } else {
        SafeRelease(&pD2DBitmap);
        return nullptr;
    }
}


HRESULT IconManager::ConvertHICONToD2DBitmap(HICON hIcon, ID2D1Bitmap** ppBitmap) {
    if (!m_pWICFactory || !m_pRenderTarget || !hIcon || !ppBitmap) return E_INVALIDARG;
    *ppBitmap = nullptr;

    ICONINFO iconInfo = {0};
    if (!GetIconInfo(hIcon, &iconInfo)) return E_FAIL;

    BITMAP bmColor = {0};
    BITMAP bmMask = {0};

    int width = 0;
    int height = 0;

    if (iconInfo.hbmColor) {
        GetObject(iconInfo.hbmColor, sizeof(bmColor), &bmColor);
        width = bmColor.bmWidth;
        height = bmColor.bmHeight;
    } else if (iconInfo.hbmMask) {
        GetObject(iconInfo.hbmMask, sizeof(bmMask), &bmMask);
        width = bmMask.bmWidth;
        height = bmMask.bmHeight / 2;
    } else {
        DeleteObject(iconInfo.hbmColor);
        DeleteObject(iconInfo.hbmMask);
        return E_FAIL;
    }

    if (width == 0 || height == 0) {
         DeleteObject(iconInfo.hbmColor);
         DeleteObject(iconInfo.hbmMask);
         return E_FAIL;
    }

    IWICBitmap* pWICBitmap = nullptr;
    HRESULT hr = m_pWICFactory->CreateBitmap(width, height, GUID_WICPixelFormat32bppPBGRA, WICBitmapCacheOnDemand, &pWICBitmap);

    if (SUCCEEDED(hr)) {
        IWICBitmapLock *pLock = nullptr;
        WICRect rcLock = {0, 0, width, height};
        hr = pWICBitmap->Lock(&rcLock, WICBitmapLockWrite, &pLock);

        if (SUCCEEDED(hr)) {
            UINT cbBufferSize = 0;
            UINT cbStride = 0;
            BYTE *pv = nullptr;

            hr = pLock->GetStride(&cbStride);
            if(SUCCEEDED(hr)) hr = pLock->GetDataPointer(&cbBufferSize, &pv);

            if (SUCCEEDED(hr)) {
                HDC hdcScreen = GetDC(NULL);
                HDC hdcMem = CreateCompatibleDC(hdcScreen);

                BITMAPINFO bi = {0};
                bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                bi.bmiHeader.biWidth = width;
                bi.bmiHeader.biHeight = -height;
                bi.bmiHeader.biPlanes = 1;
                bi.bmiHeader.biBitCount = 32;
                bi.bmiHeader.biCompression = BI_RGB;
                HBITMAP hbmTarget = CreateDIBSection(hdcMem, &bi, DIB_RGB_COLORS, (void**)pv, NULL, 0);

                if (hbmTarget) {
                    HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmTarget);
                    memset(pv, 0, cbBufferSize);
                    DrawIconEx(hdcMem, 0, 0, hIcon, width, height, 0, NULL, DI_NORMAL);
                    SelectObject(hdcMem, hbmOld);
                    DeleteObject(hbmTarget);
                } else {
                    hr = E_OUTOFMEMORY;
                }
                DeleteDC(hdcMem);
                ReleaseDC(NULL, hdcScreen);
            }
            SafeRelease(&pLock);
        }

        if (SUCCEEDED(hr)) {
            hr = m_pRenderTarget->CreateBitmapFromWicBitmap(pWICBitmap, nullptr, ppBitmap);
        }
        SafeRelease(&pWICBitmap);
    }

    DeleteObject(iconInfo.hbmColor);
    DeleteObject(iconInfo.hbmMask);

    return hr;
}
