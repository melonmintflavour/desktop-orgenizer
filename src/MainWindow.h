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

class DatabaseManager; // Forward declaration

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

    // UI Action for adding a zone
    void addZoneToCurrentPage();


protected:
    void paintEvent(QPaintEvent *event) override;
    void closeEvent(QCloseEvent *event) override; // For saving on close
    // void mousePressEvent(QMouseEvent *event) override; // Will be handled by child widgets or specific areas
    // void mouseMoveEvent(QMouseEvent *event) override;
    // void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void setupUI();
    void setupPageControls();
    void loadSettings();
    void saveSettings();

    QPoint m_dragPosition; // Keep for now, might be useful for dragging toolbar/main window parts

    PageManager* m_pageManager;
    DatabaseManager* m_dbManager; // Database manager instance
    QTabWidget* m_tabWidget;
    QPushButton* m_addPageButton;
    QPushButton* m_addZoneButton; // Button to add a new zone
    QWidget* m_centralWidget; // Main container for layout
    QVBoxLayout* m_mainLayout; // Main vertical layout
    QHBoxLayout* m_controlsLayout; // Layout for controls like add page button
    QWidget* m_pageControlsWidget; // Widget to hold page controls
};

#endif // MAINWINDOW_H
