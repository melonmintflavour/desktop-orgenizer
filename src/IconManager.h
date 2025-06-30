#ifndef ICONMANAGER_H
#define ICONMANAGER_H

#include <windows.h>
#include <d2d1.h>
#include <wincodec.h>
#include <string>
#include <map>
#include "DataModels.h"

template<class Interface>
inline void SafeRelease(Interface **ppInterfaceToRelease); // Ensure this is declared or included if used here

class IconManager {
public:
    IconManager();
    ~IconManager();

    HRESULT Initialize(ID2D1RenderTarget* pRT);
    void Shutdown();

    ID2D1Bitmap* GetIconBitmap(ns::DesktopIcon& iconInfo);
    ID2D1Bitmap* LoadImageFileAsBitmap(const std::wstring& imagePath);

    void ReleaseDeviceResources();

// Make m_pWICFactory accessible for main.cpp's OnPaint temporary image loading.
// This is not ideal; a better solution would be to pass IWICImagingFactory* to functions needing it,
// or have IconManager handle all WIC-based loading. For now, this is a pragmatic step.
// Update: Decided against making it public. Will create a helper or extend IconManager.
// For now, LoadImageFileAsBitmap will be part of IconManager.
// public: // Temporarily public for main.cpp's OnPaint, to be refactored.
    IWICImagingFactory* m_pWICFactory;

private:
    HRESULT ConvertHICONToD2DBitmap(HICON hIcon, ID2D1Bitmap** ppBitmap);

    ID2D1RenderTarget* m_pRenderTarget;
    // IWICImagingFactory* m_pWICFactory; // Now public for temporary access

    std::map<std::wstring, ID2D1Bitmap*> m_iconBitmapCache;
    std::map<std::wstring, ID2D1Bitmap*> m_genericImageCache;
};

#endif // ICONMANAGER_H
