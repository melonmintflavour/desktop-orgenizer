#ifndef HUBWINDOW_H
#define HUBWINDOW_H

#define UNICODE
#define _UNICODE

#include <windows.h>
#include "DataModels.h"
#include <vector>
#include <string>

class PersistenceManager;

extern ns::ApplicationSettings appSettings;
extern PersistenceManager* pPersistenceManager;
extern HWND g_hwndMainOverlay;

class HubWindow {
public:
    HubWindow(HINSTANCE hInstance);
    ~HubWindow();

    bool Create();
    void Show(int nCmdShow);
    HWND GetHwnd() const { return m_hwnd; }
    void LoadSettingsToListControls();

private:
    static LRESULT CALLBACK HubWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

    void InitializeControls();
    void PopulatePageList();
    void PopulateZoneList(const std::wstring& pageId);
    void PopulateZoneProperties(const ns::IconZone* zone);
    void ClearZoneProperties();
    ns::IconZone* GetSelectedZone(); // Helper to get current zone from appSettings

    void OnAddPage();
    void OnRemovePage();
    void OnRenamePage();

    void OnAddZone();
    void OnRemoveZone();
    void OnApplyZoneChanges();
    void OnBrowseBackgroundImage();

    // Icon Organization
    void OnSortKeyChanged();
    void OnSortOrderChanged();
    void OnSearchTermChanged();
    void SortIconsInSelectedZone(); // Renamed from SortIconsInZone to be more specific
    void ReflowIconsInZone(ns::IconZone* zone); // Keep this for direct use if needed

    HINSTANCE m_hInstance;
    HWND m_hwnd;
    HWND m_hwndPageList;
    HWND m_hwndZoneList;
    HWND m_hwndAddPageButton;
    HWND m_hwndRemovePageButton;
    HWND m_hwndRenamePageButton;
    HWND m_hwndAddZoneButton;
    HWND m_hwndRemoveZoneButton;

    HWND m_hwndZonePropertiesGroup;
    HWND m_hwndZoneTitleLabel;
    HWND m_hwndZoneTitleEdit;
    HWND m_hwndBgTypeLabel;
    HWND m_hwndBgTypeCombo;
    HWND m_hwndBgColorLabel;
    HWND m_hwndBgColorRedEdit;
    HWND m_hwndBgColorGreenEdit;
    HWND m_hwndBgColorBlueEdit;
    HWND m_hwndBgColorAlphaEdit;
    HWND m_hwndBgImagePathLabel;
    HWND m_hwndBgImagePathEdit;
    HWND m_hwndBrowseBgImageButton;
    HWND m_hwndGridRowsLabel;
    HWND m_hwndGridRowsEdit;
    HWND m_hwndGridColsLabel;
    HWND m_hwndGridColsEdit;
    HWND m_hwndApplyZoneChangesButton;

    // Controls for Zone Icon Organization
    HWND m_hwndSortIconsLabel;
    HWND m_hwndSortKeyCombo;
    HWND m_hwndSortOrderButton;
    HWND m_hwndSearchIconLabel;
    HWND m_hwndSearchIconEdit;

    std::wstring m_selectedPageId;
    std::wstring m_selectedZoneId;
    std::wstring m_currentIconSearchTerm; // Stores current search term for filtering in OnPaint
};

#endif // HUBWINDOW_H
