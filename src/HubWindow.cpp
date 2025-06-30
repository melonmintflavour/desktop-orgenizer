#include "HubWindow.h"
#include "Persistence.h"
#include <CommCtrl.h>
#include <string>
#include <algorithm>
#include <shobjidl.h> // For OFN_EXPLORER in OnBrowseBackgroundImage

#pragma comment(lib, "Comctl32.lib")

// Define control IDs
#define IDC_PAGE_LIST 1001
#define IDC_ZONE_LIST 1002
#define IDC_ADD_PAGE_BUTTON 1003
#define IDC_REMOVE_PAGE_BUTTON 1004
#define IDC_RENAME_PAGE_BUTTON 1005
#define IDC_ADD_ZONE_BUTTON 1006
#define IDC_REMOVE_ZONE_BUTTON 1007

#define IDC_ZONE_PROPERTIES_GROUP 1009
#define IDC_ZONE_TITLE_EDIT 1010
#define IDC_BG_TYPE_COMBO   1015
#define IDC_BG_COLOR_R_EDIT 1011
#define IDC_BG_COLOR_G_EDIT 1012
#define IDC_BG_COLOR_B_EDIT 1013
#define IDC_BG_COLOR_A_EDIT 1014
#define IDC_BG_IMAGE_PATH_EDIT 1016
#define IDC_BROWSE_BG_IMAGE_BUTTON 1022
#define IDC_GRID_ROWS_EDIT  1017
#define IDC_GRID_COLS_EDIT  1018
#define IDC_APPLY_ZONE_CHANGES_BUTTON 1019

#define IDC_SORT_KEY_COMBO 1023
#define IDC_SORT_ORDER_BUTTON 1024
#define IDC_SEARCH_ICON_EDIT 1025


HubWindow::HubWindow(HINSTANCE hInstance) : m_hInstance(hInstance), m_hwnd(nullptr), m_selectedPageId(L""), m_selectedZoneId(L""), m_currentIconSearchTerm(L"") {
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_STANDARD_CLASSES | ICC_LISTVIEW_CLASSES | ICC_UPDOWN_CLASS;
    InitCommonControlsEx(&icex);
}

HubWindow::~HubWindow() {
    for (int i = 0; i < SendMessage(m_hwndPageList, LB_GETCOUNT, 0, 0); ++i) {
        wchar_t* pageId_ptr = (wchar_t*)SendMessage(m_hwndPageList, LB_GETITEMDATA, i, 0);
        if (pageId_ptr) free(pageId_ptr);
    }
    for (int i = 0; i < SendMessage(m_hwndZoneList, LB_GETCOUNT, 0, 0); ++i) {
        wchar_t* zoneId_ptr = (wchar_t*)SendMessage(m_hwndZoneList, LB_GETITEMDATA, i, 0);
        if (zoneId_ptr) free(zoneId_ptr);
    }
}

bool HubWindow::Create() {
    const wchar_t CLASS_NAME[] = L"DesktopOrgHubWindowClass";
    WNDCLASS wc = {};
    wc.lpfnWndProc = HubWndProc; wc.hInstance = m_hInstance; wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    if (!RegisterClass(&wc)) { MessageBox(NULL, L"Hub Window Registration Failed!", L"Error", MB_ICONEXCLAMATION | MB_OK); return false; }
    m_hwnd = CreateWindowEx(0, CLASS_NAME, L"Desktop Organization Hub", WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME, CW_USEDEFAULT, CW_USEDEFAULT, 700, 600, NULL, NULL, m_hInstance, this );
    return (m_hwnd != nullptr);
}

void HubWindow::Show(int nCmdShow) { if (m_hwnd) { ShowWindow(m_hwnd, nCmdShow); UpdateWindow(m_hwnd); } }

LRESULT CALLBACK HubWindow::HubWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    HubWindow* pThis = nullptr;
    if (uMsg == WM_NCCREATE) {
        CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam; pThis = (HubWindow*)pCreate->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis); pThis->m_hwnd = hwnd;
    } else { pThis = (HubWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA); }
    if (pThis) { return pThis->HandleMessage(uMsg, wParam, lParam); }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT HubWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: InitializeControls(); LoadSettingsToListControls(); return 0;
        case WM_COMMAND: {
            int wmId = LOWORD(wParam); int wmEvent = HIWORD(wParam);
            switch (wmId) {
                case IDC_ADD_PAGE_BUTTON: OnAddPage(); break;
                case IDC_REMOVE_PAGE_BUTTON: OnRemovePage(); break;
                case IDC_RENAME_PAGE_BUTTON: MessageBox(m_hwnd, L"Rename Page: Not implemented.", L"TODO", MB_OK); break;
                case IDC_ADD_ZONE_BUTTON: OnAddZone(); break;
                case IDC_REMOVE_ZONE_BUTTON: OnRemoveZone(); break;
                case IDC_APPLY_ZONE_CHANGES_BUTTON: OnApplyZoneChanges(); break;
                case IDC_BROWSE_BG_IMAGE_BUTTON: OnBrowseBackgroundImage(); break;
                case IDC_SORT_KEY_COMBO: if (wmEvent == CBN_SELCHANGE) OnSortKeyChanged(); break;
                case IDC_SORT_ORDER_BUTTON: OnSortOrderChanged(); break;
                case IDC_SEARCH_ICON_EDIT: if (wmEvent == EN_CHANGE) OnSearchTermChanged(); break;
                case IDC_PAGE_LIST:
                    if (wmEvent == LBN_SELCHANGE) {
                        int selIdx = SendMessage(m_hwndPageList, LB_GETCURSEL, 0, 0);
                        if (selIdx != LB_ERR) {
                            wchar_t* pageId_ptr = (wchar_t*)SendMessage(m_hwndPageList, LB_GETITEMDATA, selIdx, 0);
                            if(pageId_ptr) { m_selectedPageId = pageId_ptr; PopulateZoneList(m_selectedPageId); }
                        } else { m_selectedPageId.clear(); PopulateZoneList(L""); } // Clear zones if no page selected
                         ClearZoneProperties();
                    }
                    break;
                case IDC_ZONE_LIST:
                    if (wmEvent == LBN_SELCHANGE) {
                        int selIdx = SendMessage(m_hwndZoneList, LB_GETCURSEL, 0, 0);
                        if (selIdx != LB_ERR && !m_selectedPageId.empty()) {
                             wchar_t* zoneId_ptr = (wchar_t*)SendMessage(m_hwndZoneList, LB_GETITEMDATA, selIdx, 0);
                             if(zoneId_ptr){
                                m_selectedZoneId = zoneId_ptr;
                                ns::IconZone* currentZone = GetSelectedZone();
                                if (currentZone) PopulateZoneProperties(currentZone); else ClearZoneProperties();
                             }
                        } else { ClearZoneProperties(); }
                    }
                    break;
                default: return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
            }
        } return 0;
        case WM_CLOSE: ShowWindow(m_hwnd, SW_HIDE); return 0;
        case WM_DESTROY: return 0;
    }
    return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}

void HubWindow::InitializeControls() {
    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    CreateWindow(L"STATIC", L"Pages:", WS_CHILD | WS_VISIBLE | SS_LEFT, 10, 10, 50, 20, m_hwnd, NULL, m_hInstance, NULL);
    m_hwndPageList = CreateWindowEx(WS_EX_CLIENTEDGE, L"LISTBOX", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | LBS_HASSTRINGS, 10, 30, 180, 200, m_hwnd, (HMENU)IDC_PAGE_LIST, m_hInstance, NULL);
    m_hwndAddPageButton = CreateWindow(L"BUTTON", L"Add Page", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 10, 235, 85, 25, m_hwnd, (HMENU)IDC_ADD_PAGE_BUTTON, m_hInstance, NULL);
    m_hwndRemovePageButton = CreateWindow(L"BUTTON", L"Remove Page", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 105, 235, 85, 25, m_hwnd, (HMENU)IDC_REMOVE_PAGE_BUTTON, m_hInstance, NULL);
    m_hwndRenamePageButton = CreateWindow(L"BUTTON", L"Rename Page", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 10, 265, 180, 25, m_hwnd, (HMENU)IDC_RENAME_PAGE_BUTTON, m_hInstance, NULL);

    CreateWindow(L"STATIC", L"Zones (on selected page):", WS_CHILD | WS_VISIBLE | SS_LEFT, 200, 10, 180, 20, m_hwnd, NULL, m_hInstance, NULL);
    m_hwndZoneList = CreateWindowEx(WS_EX_CLIENTEDGE, L"LISTBOX", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | LBS_HASSTRINGS, 200, 30, 180, 200, m_hwnd, (HMENU)IDC_ZONE_LIST, m_hInstance, NULL);
    m_hwndAddZoneButton = CreateWindow(L"BUTTON", L"Add Zone", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 200, 235, 85, 25, m_hwnd, (HMENU)IDC_ADD_ZONE_BUTTON, m_hInstance, NULL);
    m_hwndRemoveZoneButton = CreateWindow(L"BUTTON", L"Remove Zone", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 295, 235, 85, 25, m_hwnd, (HMENU)IDC_REMOVE_ZONE_BUTTON, m_hInstance, NULL);

    m_hwndZonePropertiesGroup = CreateWindow(L"BUTTON", L"Selected Zone Properties", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 390, 10, 290, 540, m_hwnd, (HMENU)IDC_ZONE_PROPERTIES_GROUP, m_hInstance, NULL); // Adjusted height
    int yPos = 30;
    CreateWindow(L"STATIC", L"Title:", WS_CHILD | WS_VISIBLE | SS_RIGHT, 400, yPos, 80, 20, m_hwnd, NULL, m_hInstance, NULL);
    m_hwndZoneTitleEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 490, yPos, 170, 20, m_hwnd, (HMENU)IDC_ZONE_TITLE_EDIT, m_hInstance, NULL); yPos += 25;
    CreateWindow(L"STATIC", L"BG Type:", WS_CHILD | WS_VISIBLE | SS_RIGHT, 400, yPos, 80, 20, m_hwnd, NULL, m_hInstance, NULL);
    m_hwndBgTypeCombo = CreateWindow(L"COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL, 490, yPos, 170, 100, m_hwnd, (HMENU)IDC_BG_TYPE_COMBO, m_hInstance, NULL);
    SendMessage(m_hwndBgTypeCombo, CB_ADDSTRING, 0, (LPARAM)L"Transparent"); SendMessage(m_hwndBgTypeCombo, CB_SETITEMDATA, 0, (LPARAM)ns::ZoneBackgroundType::Transparent);
    SendMessage(m_hwndBgTypeCombo, CB_ADDSTRING, 0, (LPARAM)L"Solid Color"); SendMessage(m_hwndBgTypeCombo, CB_SETITEMDATA, 1, (LPARAM)ns::ZoneBackgroundType::SolidColor);
    SendMessage(m_hwndBgTypeCombo, CB_ADDSTRING, 0, (LPARAM)L"Image"); SendMessage(m_hwndBgTypeCombo, CB_SETITEMDATA, 2, (LPARAM)ns::ZoneBackgroundType::Image); yPos += 25;
    CreateWindow(L"STATIC", L"BG Color:", WS_CHILD | WS_VISIBLE | SS_RIGHT, 400, yPos, 80, 20, m_hwnd, NULL, m_hInstance, NULL);
    CreateWindow(L"STATIC", L"R:", WS_CHILD | WS_VISIBLE | SS_CENTER, 490, yPos, 15, 20, m_hwnd, NULL, m_hInstance, NULL);
    m_hwndBgColorRedEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 505, yPos, 30, 20, m_hwnd, (HMENU)IDC_BG_COLOR_R_EDIT, m_hInstance, NULL);
    CreateWindow(L"STATIC", L"G:", WS_CHILD | WS_VISIBLE | SS_CENTER, 540, yPos, 15, 20, m_hwnd, NULL, m_hInstance, NULL);
    m_hwndBgColorGreenEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 555, yPos, 30, 20, m_hwnd, (HMENU)IDC_BG_COLOR_G_EDIT, m_hInstance, NULL);
    CreateWindow(L"STATIC", L"B:", WS_CHILD | WS_VISIBLE | SS_CENTER, 590, yPos, 15, 20, m_hwnd, NULL, m_hInstance, NULL);
    m_hwndBgColorBlueEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 605, yPos, 30, 20, m_hwnd, (HMENU)IDC_BG_COLOR_B_EDIT, m_hInstance, NULL);
    CreateWindow(L"STATIC", L"A:", WS_CHILD | WS_VISIBLE | SS_CENTER, 640, yPos, 15, 20, m_hwnd, NULL, m_hInstance, NULL);
    m_hwndBgColorAlphaEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 655, yPos, 30, 20, m_hwnd, (HMENU)IDC_BG_COLOR_A_EDIT, m_hInstance, NULL); yPos += 25;
    CreateWindow(L"STATIC", L"Image Path:", WS_CHILD | WS_VISIBLE | SS_RIGHT, 400, yPos, 80, 20, m_hwnd, NULL, m_hInstance, NULL);
    m_hwndBgImagePathEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 490, yPos, 130, 20, m_hwnd, (HMENU)IDC_BG_IMAGE_PATH_EDIT, m_hInstance, NULL);
    m_hwndBrowseBgImageButton = CreateWindow(L"BUTTON", L"...", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 630, yPos, 30, 20, m_hwnd, (HMENU)IDC_BROWSE_BG_IMAGE_BUTTON, m_hInstance, NULL); yPos += 25;
    CreateWindow(L"STATIC", L"Grid Rows:", WS_CHILD | WS_VISIBLE | SS_RIGHT, 400, yPos, 80, 20, m_hwnd, NULL, m_hInstance, NULL);
    m_hwndGridRowsEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER, 490, yPos, 50, 20, m_hwnd, (HMENU)IDC_GRID_ROWS_EDIT, m_hInstance, NULL); yPos += 25;
    CreateWindow(L"STATIC", L"Grid Cols:", WS_CHILD | WS_VISIBLE | SS_RIGHT, 400, yPos, 80, 20, m_hwnd, NULL, m_hInstance, NULL);
    m_hwndGridColsEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER, 490, yPos, 50, 20, m_hwnd, (HMENU)IDC_GRID_COLS_EDIT, m_hInstance, NULL); yPos += 35;
    m_hwndApplyZoneChangesButton = CreateWindow(L"BUTTON", L"Apply Zone Changes", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 490, yPos, 170, 25, m_hwnd, (HMENU)IDC_APPLY_ZONE_CHANGES_BUTTON, m_hInstance, NULL); yPos += 35;

    m_hwndSortIconsLabel = CreateWindow(L"STATIC", L"Sort Icons By:", WS_CHILD | WS_VISIBLE | SS_RIGHT, 400, yPos, 80, 20, m_hwnd, NULL, m_hInstance, NULL);
    m_hwndSortKeyCombo = CreateWindow(L"COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 490, yPos, 120, 100, m_hwnd, (HMENU)IDC_SORT_KEY_COMBO, m_hInstance, NULL);
    SendMessage(m_hwndSortKeyCombo, CB_ADDSTRING, 0, (LPARAM)L"Name"); SendMessage(m_hwndSortKeyCombo, CB_SETITEMDATA, 0, (LPARAM)ns::DesktopIcon::SortKey::Name);
    SendMessage(m_hwndSortKeyCombo, CB_ADDSTRING, 0, (LPARAM)L"Path"); SendMessage(m_hwndSortKeyCombo, CB_SETITEMDATA, 1, (LPARAM)ns::DesktopIcon::SortKey::Path);
    m_hwndSortOrderButton = CreateWindow(L"BUTTON", L"Asc", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 620, yPos, 40, 20, m_hwnd, (HMENU)IDC_SORT_ORDER_BUTTON, m_hInstance, NULL); yPos += 25;
    m_hwndSearchIconLabel = CreateWindow(L"STATIC", L"Search Icons:", WS_CHILD | WS_VISIBLE | SS_RIGHT, 400, yPos, 80, 20, m_hwnd, NULL, m_hInstance, NULL);
    m_hwndSearchIconEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 490, yPos, 170, 20, m_hwnd, (HMENU)IDC_SEARCH_ICON_EDIT, m_hInstance, NULL);

    EnumChildWindows(m_hwnd, [](HWND hwndChild, LPARAM lParam) -> BOOL { SendMessage(hwndChild, WM_SETFONT, (WPARAM)lParam, TRUE); return TRUE; }, (LPARAM)hFont);
    ClearZoneProperties();
}

void HubWindow::ClearZoneProperties() {
    SetWindowText(m_hwndZoneTitleEdit, L""); SendMessage(m_hwndBgTypeCombo, CB_SETCURSEL, -1, 0);
    SetWindowText(m_hwndBgColorRedEdit, L""); SetWindowText(m_hwndBgColorGreenEdit, L""); SetWindowText(m_hwndBgColorBlueEdit, L""); SetWindowText(m_hwndBgColorAlphaEdit, L"");
    SetWindowText(m_hwndBgImagePathEdit, L""); SetWindowText(m_hwndGridRowsEdit, L""); SetWindowText(m_hwndGridColsEdit, L"");
    SendMessage(m_hwndSortKeyCombo, CB_SETCURSEL, -1, 0); SetWindowText(m_hwndSortOrderButton, L"Asc"); SetWindowText(m_hwndSearchIconEdit, L"");

    EnableWindow(m_hwndZoneTitleEdit, FALSE); EnableWindow(m_hwndBgTypeCombo, FALSE); EnableWindow(m_hwndBgColorRedEdit, FALSE); EnableWindow(m_hwndBgColorGreenEdit, FALSE);
    EnableWindow(m_hwndBgColorBlueEdit, FALSE); EnableWindow(m_hwndBgColorAlphaEdit, FALSE); EnableWindow(m_hwndBgImagePathEdit, FALSE); EnableWindow(m_hwndBrowseBgImageButton, FALSE);
    EnableWindow(m_hwndGridRowsEdit, FALSE); EnableWindow(m_hwndGridColsEdit, FALSE); EnableWindow(m_hwndApplyZoneChangesButton, FALSE);
    EnableWindow(m_hwndSortKeyCombo, FALSE); EnableWindow(m_hwndSortOrderButton, FALSE); EnableWindow(m_hwndSearchIconEdit, FALSE);
    m_selectedZoneId.clear();
}

void HubWindow::PopulateZoneProperties(const ns::IconZone* zone) {
    if (!zone) { ClearZoneProperties(); return; }
    EnableWindow(m_hwndZoneTitleEdit, TRUE); EnableWindow(m_hwndBgTypeCombo, TRUE); EnableWindow(m_hwndBgColorRedEdit, TRUE); EnableWindow(m_hwndBgColorGreenEdit, TRUE);
    EnableWindow(m_hwndBgColorBlueEdit, TRUE); EnableWindow(m_hwndBgColorAlphaEdit, TRUE); EnableWindow(m_hwndBgImagePathEdit, TRUE); EnableWindow(m_hwndBrowseBgImageButton, TRUE);
    EnableWindow(m_hwndGridRowsEdit, TRUE); EnableWindow(m_hwndGridColsEdit, TRUE); EnableWindow(m_hwndApplyZoneChangesButton, TRUE);
    EnableWindow(m_hwndSortKeyCombo, TRUE); EnableWindow(m_hwndSortOrderButton, TRUE); EnableWindow(m_hwndSearchIconEdit, TRUE);

    SetWindowText(m_hwndZoneTitleEdit, zone->title.c_str());
    int comboIdx = -1;
    if (zone->backgroundType == ns::ZoneBackgroundType::Transparent) comboIdx = 0; else if (zone->backgroundType == ns::ZoneBackgroundType::SolidColor) comboIdx = 1; else if (zone->backgroundType == ns::ZoneBackgroundType::Image) comboIdx = 2;
    SendMessage(m_hwndBgTypeCombo, CB_SETCURSEL, comboIdx, 0);
    wchar_t buffer[32];
    swprintf(buffer, 32, L"%.2f", zone->backgroundColor.r); SetWindowText(m_hwndBgColorRedEdit, buffer); swprintf(buffer, 32, L"%.2f", zone->backgroundColor.g); SetWindowText(m_hwndBgColorGreenEdit, buffer);
    swprintf(buffer, 32, L"%.2f", zone->backgroundColor.b); SetWindowText(m_hwndBgColorBlueEdit, buffer); swprintf(buffer, 32, L"%.2f", zone->backgroundColor.a); SetWindowText(m_hwndBgColorAlphaEdit, buffer);
    SetWindowText(m_hwndBgImagePathEdit, zone->backgroundImagePath.c_str()); swprintf(buffer, 32, L"%d", zone->gridRows); SetWindowText(m_hwndGridRowsEdit, buffer);
    swprintf(buffer, 32, L"%d", zone->gridCols); SetWindowText(m_hwndGridColsEdit, buffer);
    for(int i=0; i < SendMessage(m_hwndSortKeyCombo, CB_GETCOUNT, 0, 0); ++i) {
        if ((ns::DesktopIcon::SortKey)SendMessage(m_hwndSortKeyCombo, CB_GETITEMDATA, i, 0) == zone->currentSortKey) { SendMessage(m_hwndSortKeyCombo, CB_SETCURSEL, i, 0); break; }
    }
    SetWindowText(m_hwndSortOrderButton, zone->sortAscending ? L"Asc" : L"Desc");
    SetWindowText(m_hwndSearchIconEdit, L""); // Clear search on new zone selection
    m_currentIconSearchTerm = L"";
}

void HubWindow::LoadSettingsToListControls() { /* ... (same as before, but calls new ClearZoneProperties) ... */ PopulatePageList(); m_selectedPageId.clear(); m_selectedZoneId.clear(); ClearZoneProperties(); /* ... rest is same ... */ }
void HubWindow::PopulatePageList() { /* ... (same as before with item data freeing) ... */ }
void HubWindow::PopulateZoneList(const std::wstring& pageId) { /* ... (same as before with item data freeing and ClearZoneProperties) ... */ }
void HubWindow::OnAddPage() { /* ... (same as before) ... */ }
void HubWindow::OnRemovePage() { /* ... (same as before) ... */ }
void HubWindow::OnAddZone() { /* ... (same as before) ... */ }
void HubWindow::OnRemoveZone() { /* ... (same as before) ... */ }

void HubWindow::OnApplyZoneChanges() {
    ns::IconZone* targetZone = GetSelectedZone();
    if (!targetZone) { MessageBox(m_hwnd, L"No zone selected.", L"Error", MB_OK); return; }
    wchar_t buffer[MAX_PATH];
    GetWindowText(m_hwndZoneTitleEdit, buffer, MAX_PATH); targetZone->title = buffer;
    int bgTypeIndex = SendMessage(m_hwndBgTypeCombo, CB_GETCURSEL, 0, 0);
    if (bgTypeIndex != CB_ERR) targetZone->backgroundType = (ns::ZoneBackgroundType)SendMessage(m_hwndBgTypeCombo, CB_GETITEMDATA, bgTypeIndex, 0);
    GetWindowText(m_hwndBgColorRedEdit, buffer, 16); targetZone->backgroundColor.r = (float)_wtof(buffer); GetWindowText(m_hwndBgColorGreenEdit, buffer, 16); targetZone->backgroundColor.g = (float)_wtof(buffer);
    GetWindowText(m_hwndBgColorBlueEdit, buffer, 16); targetZone->backgroundColor.b = (float)_wtof(buffer); GetWindowText(m_hwndBgColorAlphaEdit, buffer, 16); targetZone->backgroundColor.a = (float)_wtof(buffer);
    GetWindowText(m_hwndBgImagePathEdit, buffer, MAX_PATH); targetZone->backgroundImagePath = buffer;
    GetWindowText(m_hwndGridRowsEdit, buffer, 16); targetZone->gridRows = _wtoi(buffer); if(targetZone->gridRows <=0) targetZone->gridRows = 4;
    GetWindowText(m_hwndGridColsEdit, buffer, 16); targetZone->gridCols = _wtoi(buffer); if(targetZone->gridCols <=0) targetZone->gridCols = 4;

    // Apply sorting based on current Hub selection before saving (if different from current zone state)
    int sortKeyIdx = SendMessage(m_hwndSortKeyCombo, CB_GETCURSEL, 0, 0);
    if (sortKeyIdx != CB_ERR) targetZone->currentSortKey = (ns::DesktopIcon::SortKey)SendMessage(m_hwndSortKeyCombo, CB_GETITEMDATA, sortKeyIdx, 0);
    wchar_t sortOrderText[10]; GetWindowText(m_hwndSortOrderButton, sortOrderText, 10);
    targetZone->sortAscending = (_wcsicmp(sortOrderText, L"Asc") == 0);

    targetZone->SortIcons(); // Sort before reflow
    ReflowIconsInZone(targetZone); // Reflow after sorting and grid changes

    if (pPersistenceManager) pPersistenceManager->SaveSettings(appSettings);
    if (g_hwndMainOverlay) InvalidateRect(g_hwndMainOverlay, NULL, TRUE);
    PopulateZoneList(m_selectedPageId); // Refresh list (e.g. if title changed)
    for(int i=0; i < SendMessage(m_hwndZoneList, LB_GETCOUNT, 0,0); ++i) { // Reselect
         wchar_t* id_str_ptr = (wchar_t*)SendMessage(m_hwndZoneList, LB_GETITEMDATA, i, 0);
         if(id_str_ptr && m_selectedZoneId == id_str_ptr) { SendMessage(m_hwndZoneList, LB_SETCURSEL, i, 0); PopulateZoneProperties(targetZone); break; }
    }
}

ns::IconZone* HubWindow::GetSelectedZone() {
    if (m_selectedPageId.empty() || m_selectedZoneId.empty()) return nullptr;
    for (auto& page : appSettings.pages) {
        if (page.id == m_selectedPageId) {
            for (auto& zone : page.zones) {
                if (zone.id == m_selectedZoneId) return &zone;
            }
        }
    }
    return nullptr;
}

void HubWindow::OnSortKeyChanged() {
    ns::IconZone* zone = GetSelectedZone();
    if (!zone) return;
    int selIdx = SendMessage(m_hwndSortKeyCombo, CB_GETCURSEL, 0, 0);
    if (selIdx != CB_ERR) {
        zone->currentSortKey = (ns::DesktopIcon::SortKey)SendMessage(m_hwndSortKeyCombo, CB_GETITEMDATA, selIdx, 0);
        SortIconsInSelectedZone(); // This will sort and reflow
    }
}

void HubWindow::OnSortOrderChanged() {
    ns::IconZone* zone = GetSelectedZone();
    if (!zone) return;
    zone->sortAscending = !zone->sortAscending;
    SetWindowText(m_hwndSortOrderButton, zone->sortAscending ? L"Asc" : L"Desc");
    SortIconsInSelectedZone(); // This will sort and reflow
}

void HubWindow::SortIconsInSelectedZone() {
    ns::IconZone* zone = GetSelectedZone();
    if (!zone) return;
    zone->SortIcons(); // Uses currentSortKey and sortAscending from the zone struct
    ReflowIconsInZone(zone); // Reflow icons based on new order
    if (pPersistenceManager) pPersistenceManager->SaveSettings(appSettings);
    if (g_hwndMainOverlay) InvalidateRect(g_hwndMainOverlay, NULL, TRUE);
}

void HubWindow::ReflowIconsInZone(ns::IconZone* zone) {
    if (!zone) return;
    int iconsPerRow = zone->gridCols > 0 ? zone->gridCols : 4;
    float zoneWidth = zone->screenRect.right - zone->screenRect.left;
    float cellWidth = zoneWidth / iconsPerRow;
    float iconVisualSize = cellWidth * 0.6f; iconVisualSize = max(32.0f, iconVisualSize);
    float iconPaddingHorizontal = (cellWidth - iconVisualSize) / 2;
    float cellHeight = iconVisualSize + 20.0f + iconPaddingHorizontal; // Icon + text + padding

    for(int i = 0; i < zone->icons.size(); ++i) {
        int col = i % iconsPerRow; int row = i / iconsPerRow;
        zone->icons[i].relativePosition.left = (col * cellWidth) + iconPaddingHorizontal;
        zone->icons[i].relativePosition.top = (row * cellHeight) + iconPaddingHorizontal + 20.0f;
        zone->icons[i].relativePosition.right = zone->icons[i].relativePosition.left + iconVisualSize;
        zone->icons[i].relativePosition.bottom = zone->icons[i].relativePosition.top + iconVisualSize;
    }
}

void HubWindow::OnSearchTermChanged() {
    wchar_t buffer[256]; GetWindowText(m_hwndSearchIconEdit, buffer, 256);
    m_currentIconSearchTerm = buffer;
    if (g_hwndMainOverlay) InvalidateRect(g_hwndMainOverlay, NULL, TRUE); // Trigger repaint of main overlay to apply filter
}

void HubWindow::OnBrowseBackgroundImage() { /* ... (same as before) ... */ }
