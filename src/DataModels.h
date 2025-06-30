#ifndef DATAMODELS_H
#define DATAMODELS_H

#include <string>
#include <vector>
#include <d2d1.h>
#include <algorithm> // Required for std::sort

namespace nlohmann {
    template<typename T, typename SFINAE>
    struct adl_serializer;
}

struct ColorRGBA {
    float r, g, b, a;
    D2D1_COLOR_F ToD2DColor() const { return D2D1::ColorF(r, g, b, a); }
    ColorRGBA(float r_ = 0.f, float g_ = 0.f, float b_ = 0.f, float a_ = 1.f) : r(r_), g(g_), b(b_), a(a_) {} // Constructor
};

struct RectF {
    float left, top, right, bottom;
    D2D1_RECT_F ToD2DRectF() const { return D2D1::RectF(left, top, right, bottom); }
};

namespace ns {

    struct DesktopIcon {
        std::wstring id;
        std::wstring name;
        std::wstring path;
        std::wstring iconPath;
        RectF relativePosition;

        ID2D1Bitmap* pBitmap = nullptr;
        bool isSelected = false;

        DesktopIcon() : relativePosition({0.f, 0.f, 0.1f, 0.1f}), pBitmap(nullptr), isSelected(false) {}

        enum class SortKey { Name, Path, Type /* TODO */, DateAdded /* TODO */ };
    };

    struct IconSortPredicate {
        DesktopIcon::SortKey key;
        bool ascending;
        IconSortPredicate(DesktopIcon::SortKey k, bool asc) : key(k), ascending(asc) {}
        bool operator()(const DesktopIcon& a, const DesktopIcon& b) const {
            int comparisonResult = 0;
            switch (key) {
                case DesktopIcon::SortKey::Name: comparisonResult = _wcsicmp(a.name.c_str(), b.name.c_str()); break;
                case DesktopIcon::SortKey::Path: comparisonResult = _wcsicmp(a.path.c_str(), b.path.c_str()); break;
                default: comparisonResult = 0; break;
            }
            return ascending ? (comparisonResult < 0) : (comparisonResult > 0);
        }
    };

    enum class ZoneBackgroundType { Transparent, SolidColor, Blurred, Image };

    struct IconZone {
        std::wstring id;
        std::wstring title;
        RectF screenRect;
        std::vector<DesktopIcon> icons;
        ZoneBackgroundType backgroundType = ZoneBackgroundType::Transparent;
        ColorRGBA backgroundColor = {0.2f, 0.2f, 0.2f, 0.5f};
        std::wstring backgroundImagePath;
        bool snapToGrid = true;
        int gridRows = 4;
        int gridCols = 4;
        DesktopIcon::SortKey currentSortKey = DesktopIcon::SortKey::Name;
        bool sortAscending = true;
        IconZone() : screenRect({100.f, 100.f, 400.f, 400.f}) {}
        void SortIcons() { std::sort(icons.begin(), icons.end(), IconSortPredicate(currentSortKey, sortAscending)); }
    };

    struct DesktopPage {
        std::wstring id;
        std::wstring name;
        int order;
        std::vector<IconZone> zones;
        std::wstring wallpaperPath;
        bool usePageWallpaper = false;
        ColorRGBA pageOverlayColor = {0.0f, 0.0f, 0.0f, 0.0f}; // e.g. for blurred overlay tint
        bool usePageOverlayColor = false;
    };

    struct ApplicationSettings {
        std::wstring currentTheme = L"Dark";
        std::vector<DesktopPage> pages;
        std::wstring activePageId;
        int version = 1;

        // Theme Colors (will be updated by HubWindow::OnThemeChanged)
        ColorRGBA themeTabInactiveColor;
        ColorRGBA themeTabActiveColor;
        ColorRGBA themeTabTextActiveColor;
        ColorRGBA themeTabTextInactiveColor;
        ColorRGBA themeZoneDefaultBgColor;
        ColorRGBA themeHubBackgroundColor;
        ColorRGBA themeHubTextColor;


        ApplicationSettings() { SetTheme(L"Dark"); /* Initialize with Dark theme by default */ }

        void SetTheme(const std::wstring& themeName) {
            currentTheme = themeName;
            if (themeName == L"Light") {
                themeTabInactiveColor = {0.85f, 0.85f, 0.85f, 0.9f};
                themeTabActiveColor = {1.0f, 1.0f, 1.0f, 0.95f};
                themeTabTextActiveColor = {0.0f, 0.0f, 0.0f, 1.0f};
                themeTabTextInactiveColor = {0.2f, 0.2f, 0.2f, 1.0f};
                themeZoneDefaultBgColor = {0.9f, 0.9f, 0.9f, 0.6f};
                themeHubBackgroundColor = {0.95f, 0.95f, 0.95f, 1.0f}; // Approximate COLOR_WINDOW
                themeHubTextColor = {0.0f, 0.0f, 0.0f, 1.0f};       // Approximate COLOR_WINDOWTEXT
            } else { // Default to Dark theme
                currentTheme = L"Dark";
                themeTabInactiveColor = {0.15f, 0.15f, 0.15f, 0.85f};
                themeTabActiveColor = {0.05f, 0.05f, 0.05f, 0.9f}; // Slightly darker active tab
                themeTabTextActiveColor = {1.0f, 1.0f, 1.0f, 1.0f};
                themeTabTextInactiveColor = {0.7f, 0.7f, 0.7f, 1.0f};
                themeZoneDefaultBgColor = {0.2f, 0.2f, 0.2f, 0.5f};
                // For dark theme, system colors for dialogs are usually dark already if OS theme is dark.
                // These are for custom drawing if ever needed, or for consistency.
                themeHubBackgroundColor = {0.1f, 0.1f, 0.1f, 1.0f};
                themeHubTextColor = {0.9f, 0.9f, 0.9f, 1.0f};
            }
             // Initialize pages if empty AFTER theme is set
            if (pages.empty()) {
                DesktopPage defaultPage;
                defaultPage.id = L"default_page_1";
                defaultPage.name = L"My Desktop";
                defaultPage.order = 0;
                IconZone defaultZone;
                defaultZone.id = L"default_zone_1";
                defaultZone.title = L"My Files";
                defaultZone.screenRect = { 50.f, 50.f, 350.f, 350.f };
                defaultZone.backgroundColor = themeZoneDefaultBgColor; // Apply theme default
                defaultPage.zones.push_back(defaultZone);
                pages.push_back(defaultPage);
                activePageId = defaultPage.id;
            } else { // Ensure existing zones get default theme color if transparent
                 for (auto& page : pages) {
                    for (auto& zone : page.zones) {
                        if (zone.backgroundType == ZoneBackgroundType::Transparent && zone.backgroundColor.a == 0.0f) {
                            // zone.backgroundColor = themeZoneDefaultBgColor; // Or keep it fully transparent based on design
                        } else if (zone.backgroundType == ZoneBackgroundType::SolidColor && zone.backgroundColor.a == 0.0f) {
                            // If it was meant to be solid but somehow alpha is 0, apply theme default
                            zone.backgroundColor = themeZoneDefaultBgColor;
                        }
                    }
                }
            }
        }
    };
}

#endif // DATAMODELS_H
