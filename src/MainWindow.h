#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QScreen>
#include <QApplication>
#include <QGuiApplication>
#include <QTabWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

class PageManager; // Forward declaration
class PageData;    // Forward declaration for PageData
class ZoneData;    // Forward declaration for ZoneData (or include if full def needed by MOC)
#include <QUuid>   // For QUuid signal/slot parameters
#include "ZoneData.h" // Include for signal/slot parameters with ZoneData*
#include "ThemeManager.h" // For theme selection

class DatabaseManager; // Forward declaration
class QActionGroup;    // For theme menu
class WidgetHostWindow; // Forward declaration
class DraggableToolbar; // Forward declaration
class ClockWidget;      // Forward declaration
class QuickAccessPanel; // Forward declaration
class TodoWidget;       // Forward declaration

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void addNewPage();
    void onActivePageChanged(PageData* page, int index);
    void onPageAddedToManager(PageData* page, int index);
    void onPageRemovedFromManager(QUuid pageId, int tabIndexToRemove);
    void handleTabCloseRequested(int index);
    void handleCurrentTabChanged(int index);

    // Zone related slots from PageManager
    void handleZoneAddedToPage(PageData* page, ZoneData* zoneData);
    void handleZoneRemovedFromPage(PageData* page, QUuid zoneId);
    void handleZoneDataChanged(ZoneData* zone);
    void handlePageNameChanged(PageData* page); // Slot for PageManager::pageNameChanged
    void handleTabMoved(int fromIndex, int toIndex); // Slot for QTabBar::tabMoved

    // UI Action for adding a zone
    void addZoneToCurrentPage();
    // UI Action for renaming a tab
    void handleTabDoubleClicked(int index);


protected:
    void paintEvent(QPaintEvent *event) override;
    void closeEvent(QCloseEvent *event) override; // For saving on close
    // void mousePressEvent(QMouseEvent *event) override; // Will be handled by child widgets or specific areas
    // void mouseMoveEvent(QMouseEvent *event) override;
    // void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void setupUI();
    void setupPageControls();
    void setupMenuBar(); // For theme menu
    void loadSettings();
    void saveSettings();
    void applyCurrentTheme(); // Helper to apply loaded theme

    // Theme actions
    void selectLightTheme();
    void selectDarkTheme();

    // Floating widget/toolbar actions
    void showNewFloatingClock();
    void showNewToolbar();
    void showQuickAccessPanel();
    void showTodoWidget();       // Slot to show/create the TodoWidget
    void handleHostedWidgetDestroyed(QObject* obj);
    void handleIconSearchTextChanged(const QString& searchText);
    void showPageContextMenu(const QPoint& point);
    void setPageWallpaper(PageData* pageData);
    void clearPageWallpaper(PageData* pageData);
    void setPageOverlayColor(PageData* pageData);
    void clearPageOverlayColor(PageData* pageData);
    void handlePagePropertiesChanged(PageData* pageData); // Slot for PageManager signal
    void handleScreenGeometryChanged(const QRect& newGeometry); // Slot for screen changes

    // Backup/Restore
    void exportSettings();
    void importSettings();


    QPoint m_dragPosition; // Keep for now, might be useful for dragging toolbar/main window parts

    PageManager* m_pageManager;
    DatabaseManager* m_dbManager; // Database manager instance
    QTabWidget* m_tabWidget;
    QPushButton* m_addPageButton;
    QPushButton* m_addZoneButton; // Button to add a new zone
    QLineEdit* m_iconSearchLineEdit; // Search bar for icons
    QWidget* m_centralWidget; // Main container for layout
    QVBoxLayout* m_mainLayout; // Main vertical layout
    QHBoxLayout* m_controlsLayout; // Layout for controls like add page button
    QWidget* m_pageControlsWidget; // Widget to hold page controls

    // Theme menu actions
    QAction* m_lightThemeAction;
    QAction* m_darkThemeAction;
    QActionGroup* m_themeActionGroup;

    // Hosted Widgets
    QList<WidgetHostWindow*> m_hostedWidgets;
};

#endif // MAINWINDOW_H
