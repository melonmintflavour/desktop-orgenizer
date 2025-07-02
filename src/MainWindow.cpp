#include "MainWindow.h"
#include "PageManager.h"
#include "PageData.h" // Required for PageData type
#include "PageTabContentWidget.h" // Include the new widget
#include "DatabaseManager.h"      // Include DatabaseManager
#include <QPainter>
#include <QMouseEvent>
#include <QCloseEvent>           // For closeEvent
#include <QScreen>
#include <QGuiApplication>
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel> // For placeholder page content
#include <QInputDialog> // For renaming pages
#include <QMessageBox>  // For confirmation dialogs
#include <QTabBar>      // For tabMoved and tabBarDoubleClicked signals
#include <QMenuBar>     // For menu bar
#include <QActionGroup> // For theme action group
#include <QSettings>    // For QSettings (used for widget persistence)
#include <QLineEdit>    // For icon search bar
#include <QFileDialog>  // For wallpaper selection

// Include headers for new widgets/windows
#include "WidgetHostWindow.h"
#include "DraggableToolbar.h"
#include "ClockWidget.h"
#include "QuickAccessPanel.h"
#include "TodoWidget.h"       // Include the new widget


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_pageManager(new PageManager(this)),
      m_dbManager(new DatabaseManager("DesktopOverlay.sqlite", this)) // Create DB Manager
{
    // It's important to set OrganizationName and ApplicationName for QSettings
    QCoreApplication::setOrganizationName("MyCompany"); // Replace as needed
    QCoreApplication::setApplicationName("DesktopOverlay");

    setupUI();
    setupPageControls();
    setupMenuBar(); // Setup menu bar including theme options
    loadSettings(); // Load settings which includes applying the theme
    applyCurrentTheme(); // Apply loaded or default theme

    // Connect PageManager signals to MainWindow slots
    connect(m_pageManager, &PageManager::pageAdded, this, &MainWindow::onPageAddedToManager);
    connect(m_pageManager, &PageManager::pageRemoved, this, &MainWindow::onPageRemovedFromManager);
    connect(m_pageManager, &PageManager::activePageChanged, this, &MainWindow::onActivePageChanged);
    connect(m_pageManager, &PageManager::pageNameChanged, this, &MainWindow::handlePageNameChanged);
    connect(m_pageManager, &PageManager::pagePropertiesChanged, this, &MainWindow::handlePagePropertiesChanged); // Connect new signal
    connect(m_pageManager, &PageManager::zoneAddedToPage, this, &MainWindow::handleZoneAddedToPage);
    connect(m_pageManager, &PageManager::zoneRemovedFromPage, this, &MainWindow::handleZoneRemovedFromPage);
    connect(m_pageManager, &PageManager::zoneDataChanged, this, &MainWindow::handleZoneDataChanged);
    // pageOrderChanged from PageManager doesn't need a slot if DB saves the current order from m_pages directly.

    // Connect QTabWidget/QTabBar signals for UI interactions
    if (m_tabWidget && m_tabWidget->tabBar()) {
        m_tabWidget->tabBar()->setContextMenuPolicy(Qt::CustomContextMenu); // Enable context menu for tab bar
        connect(m_tabWidget->tabBar(), &QTabBar::customContextMenuRequested, this, &MainWindow::showPageContextMenu);
        connect(m_tabWidget->tabBar(), &QTabBar::tabMoved, this, &MainWindow::handleTabMoved);
        connect(m_tabWidget->tabBar(), &QTabBar::tabBarDoubleClicked, this, &MainWindow::handleTabDoubleClicked);
    }

    // Connect screen geometry changes
    QScreen* primaryScreen = QGuiApplication::primaryScreen();
    if (primaryScreen) {
        connect(primaryScreen, &QScreen::geometryChanged, this, &MainWindow::handleScreenGeometryChanged);
    } else {
        qWarning() << "Could not get primary screen to connect geometryChanged signal.";
    }


    // Default page creation is now handled in loadSettings()
}

MainWindow::~MainWindow()
{
    // m_pageManager and m_dbManager are children of MainWindow, Qt should handle their deletion.
    // saveSettings(); // Could also save here, but closeEvent is usually better for GUI apps.
}

void MainWindow::setupUI()
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);

    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        setGeometry(screen->geometry());
    } else {
        qWarning("Primary screen not found. Setting default size.");
        resize(1024, 768); // Fallback size
    }

    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);

    m_mainLayout = new QVBoxLayout(m_centralWidget);
    m_mainLayout->setContentsMargins(5,5,5,5); // Small margin to see window edges
    m_centralWidget->setLayout(m_mainLayout);
}

void MainWindow::setupPageControls()
{
    m_pageControlsWidget = new QWidget(this); // Parent to main window for styling if needed
    m_controlsLayout = new QHBoxLayout(m_pageControlsWidget);
    m_controlsLayout->setContentsMargins(0,0,0,0);

    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabsClosable(true); // Allow closing tabs
    m_tabWidget->setMovable(true);      // Allow reordering tabs
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &MainWindow::handleCurrentTabChanged);
    connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::handleTabCloseRequested);

    m_addPageButton = new QPushButton("+", this);
    m_addPageButton->setToolTip("Add New Page");
    m_addPageButton->setFixedSize(30, 30); // Small square button
    connect(m_addPageButton, &QPushButton::clicked, this, &MainWindow::addNewPage);

    m_addZoneButton = new QPushButton("Add Zone", this);
    m_addZoneButton->setToolTip("Add New Zone to Current Page");
    m_addZoneButton->setFixedHeight(30);
    connect(m_addZoneButton, &QPushButton::clicked, this, &MainWindow::addZoneToCurrentPage);

    m_iconSearchLineEdit = new QLineEdit(this);
    m_iconSearchLineEdit->setPlaceholderText("Search items on page...");
    m_iconSearchLineEdit->setClearButtonEnabled(true);
    m_iconSearchLineEdit->setFixedHeight(30);
    m_iconSearchLineEdit->setMaximumWidth(300); // Prevent it from taking too much space
    connect(m_iconSearchLineEdit, &QLineEdit::textChanged, this, &MainWindow::handleIconSearchTextChanged);


    m_controlsLayout->addWidget(m_tabWidget, 1); // Tab widget takes most space
    m_controlsLayout->addWidget(m_iconSearchLineEdit); // Add search bar
    m_controlsLayout->addWidget(m_addPageButton);
    m_controlsLayout->addWidget(m_addZoneButton);

    m_pageControlsWidget->setLayout(m_controlsLayout);

    // Add the controls widget to the top of the main layout
    m_mainLayout->addWidget(m_pageControlsWidget);

    // Add a stretch to push content below tabs (actual page content will go here)
    m_mainLayout->addStretch(1);

    // Style the tab bar and button a bit for visibility on transparent background
    // This styling is now handled by ThemeManager and global stylesheets.
    // m_pageControlsWidget->setStyleSheet(...);
    m_pageControlsWidget->setObjectName("pageControlsWidget"); // For specific styling via stylesheet
    m_addPageButton->setObjectName("addPageButton");
}

void MainWindow::setupMenuBar()
{
    QMenu *settingsMenu = menuBar()->addMenu(tr("&Settings"));
    QMenu *viewMenu = menuBar()->addMenu(tr("&View")); // Or use existing "Settings" or new "Widgets"

    // Theme submenu (in Settings)
    QMenu *themeMenu = settingsMenu->addMenu(tr("&Theme"));
    m_themeActionGroup = new QActionGroup(this);
    m_themeActionGroup->setExclusive(true);

    m_lightThemeAction = new QAction(tr("&Light"), this);
    m_lightThemeAction->setCheckable(true);
    connect(m_lightThemeAction, &QAction::triggered, this, &MainWindow::selectLightTheme);
    themeMenu->addAction(m_lightThemeAction);
    m_themeActionGroup->addAction(m_lightThemeAction);

    m_darkThemeAction = new QAction(tr("&Dark"), this);
    m_darkThemeAction->setCheckable(true);
    connect(m_darkThemeAction, &QAction::triggered, this, &MainWindow::selectDarkTheme);
    themeMenu->addAction(m_darkThemeAction);
    m_themeActionGroup->addAction(m_darkThemeAction);

    // Widgets submenu (in View)
    QMenu *widgetsMenu = viewMenu->addMenu(tr("&Widgets"));
    QAction *addClockAction = widgetsMenu->addAction(tr("Show Floating &Clock"));
    connect(addClockAction, &QAction::triggered, this, &MainWindow::showNewFloatingClock);

    QAction *addToolbarAction = widgetsMenu->addAction(tr("Show &Toolbar"));
    connect(addToolbarAction, &QAction::triggered, this, &MainWindow::showNewToolbar);

    QAction *addQuickAccessAction = widgetsMenu->addAction(tr("Show &Quick Access Panel"));
    connect(addQuickAccessAction, &QAction::triggered, this, &MainWindow::showQuickAccessPanel);

    QAction *addTodoAction = widgetsMenu->addAction(tr("Show To-Do &List"));
    connect(addTodoAction, &QAction::triggered, this, &MainWindow::showTodoWidget);

    settingsMenu->addSeparator();
    QAction *exportAction = settingsMenu->addAction(tr("&Export Settings..."));
    connect(exportAction, &QAction::triggered, this, &MainWindow::exportSettings);
    QAction *importAction = settingsMenu->addAction(tr("&Import Settings..."));
    connect(importAction, &QAction::triggered, this, &MainWindow::importSettings);


    // Help Menu (example)
    // QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    // QAction *aboutAct = helpMenu->addAction(tr("&About"), this, &MainWindow::about);
    // QAction *aboutQtAct = helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
}

void MainWindow::applyCurrentTheme() {
    ThemeManager::Theme theme = ThemeManager::currentTheme();
    if (theme == ThemeManager::Theme::Dark) {
        m_darkThemeAction->setChecked(true);
    } else {
        m_lightThemeAction->setChecked(true);
    }
    ThemeManager::applyTheme(theme); // Apply the actual stylesheet
}

void MainWindow::selectLightTheme() {
    ThemeManager::setCurrentTheme(ThemeManager::Theme::Light);
    // applyCurrentTheme(); // setCurrentTheme now calls applyTheme and saves
}

void MainWindow::selectDarkTheme() {
    ThemeManager::setCurrentTheme(ThemeManager::Theme::Dark);
    // applyCurrentTheme();
}


void MainWindow::addNewPage()
{
    QString newPageName = QString("Page %1").arg(m_pageManager->pageCount() + 1);
    m_pageManager->addPage(newPageName);
    // The onPageAddedToManager slot will handle adding the tab
}

void MainWindow::onPageAddedToManager(PageData* page, int index)
{
    if (!page) return;

    // Each tab will hold an instance of PageTabContentWidget.
    // Pass PageData and PageManager to PageTabContentWidget
    PageTabContentWidget* pageContentWidget = new PageTabContentWidget(page, m_pageManager, m_tabWidget);

    int tabIndex = m_tabWidget->addTab(pageContentWidget, page->name());

    // If this is the page that should be active (according to manager), set current tab
    if (m_pageManager->activePage() == page) {
        m_tabWidget->setCurrentIndex(tabIndex);
    }
    qDebug() << "Added tab for page:" << page->name() << "at index" << tabIndex;
}

void MainWindow::onPageRemovedFromManager(QUuid pageId, int managerIndex)
{
    Q_UNUSED(managerIndex); // managerIndex might not match tabIndex if tabs were reordered
    for (int i = 0; i < m_tabWidget->count(); ++i) {
        PageTabContentWidget* tabContent = qobject_cast<PageTabContentWidget*>(m_tabWidget->widget(i));
        if (tabContent && tabContent->pageId() == pageId) {
            m_tabWidget->removeTab(i);
            qDebug() << "Removed tab for page ID:" << pageId;
            break;
        }
    }
}

// --- Icon Search ---
void MainWindow::handleIconSearchTextChanged(const QString& searchText)
{
    PageTabContentWidget* currentTabContent = qobject_cast<PageTabContentWidget*>(m_tabWidget->currentWidget());
    if (currentTabContent) {
        currentTabContent->filterIcons(searchText);
    }
    // No action if no current valid tab content.
}

void MainWindow::onActivePageChanged(PageData* page, int index)
{
    if (page) {
        qDebug() << "Active page changed in manager:" << page->name() << "at index" << index;
        // Find the tab corresponding to this page and set it as current
        for (int i = 0; i < m_tabWidget->count(); ++i) {
            PageTabContentWidget* tabContent = qobject_cast<PageTabContentWidget*>(m_tabWidget->widget(i));
            if (tabContent && tabContent->pageId() == page->id()) {
                if (m_tabWidget->currentIndex() != i) {
                    m_tabWidget->setCurrentIndex(i); // This will trigger handleCurrentTabChanged
                }
                break;
            }
        }
    } else {
        qDebug() << "No active page in manager.";
        // m_tabWidget->setCurrentIndex(-1); // No tab selected
    }
}


// --- Icon Search ---
void MainWindow::handleIconSearchTextChanged(const QString& searchText)
{
    PageTabContentWidget* currentTabContent = qobject_cast<PageTabContentWidget*>(m_tabWidget->currentWidget());
    if (currentTabContent) {
        currentTabContent->filterIcons(searchText);
    } else {
        // If no tab is current (e.g. all closed), or if current widget is not PageTabContentWidget
        // Potentially clear search or disable search bar if no page is active/valid
        // For now, do nothing if no valid page content.
    }
}


// --- Settings Load/Save ---
void MainWindow::handleTabCloseRequested(int index)
{
    if (index < 0 || index >= m_tabWidget->count()) return;

    PageTabContentWidget* tabContent = qobject_cast<PageTabContentWidget*>(m_tabWidget->widget(index));
    if (tabContent) {
        QUuid pageId = tabContent->pageId();
        PageData* pageData = m_pageManager->pageById(pageId); // Get PageData for name
        QString pageName = pageData ? pageData->name() : "this page";

        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Confirm Delete",
                                      QString("Are you sure you want to delete '%1'?\nAll zones and icons on this page will be lost.").arg(pageName),
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            qDebug() << "Tab close confirmed for page ID:" << pageId << "at tab index" << index;
            m_pageManager->removePageById(pageId); // This will trigger onPageRemovedFromManager
        } else {
            qDebug() << "Tab close cancelled for page ID:" << pageId;
        }
    }
}


// --- Backup and Restore ---
void MainWindow::exportSettings() {
    // 1. Ensure current settings are saved to their respective files
    saveSettings(); // This saves SQLite DB and QSettings for hosted widgets

    // 2. Get paths
    QString sqliteDbPath = m_dbManager->databasePath();
    QSettings qSettings; // Create a temporary QSettings to get its file path
    QString qSettingsPath = qSettings.fileName();

    if (sqliteDbPath.isEmpty() || qSettingsPath.isEmpty()) {
        QMessageBox::critical(this, "Export Error", "Could not determine settings file paths.");
        return;
    }

    // 3. Prompt for backup directory
    QString backupDir = QFileDialog::getExistingDirectory(this, "Select Backup Folder",
                                                        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    if (backupDir.isEmpty()) {
        return; // User cancelled
    }

    // 4. Construct target paths and copy
    QFileInfo dbFileInfo(sqliteDbPath);
    QString targetSqlitePath = backupDir + "/" + dbFileInfo.fileName();

    QFileInfo qSettingsFileInfo(qSettingsPath);
    QString targetQSettingsPath = backupDir + "/" + qSettingsFileInfo.fileName();

    bool dbCopied = QFile::copy(sqliteDbPath, targetSqlitePath);
    if (!dbCopied) {
        // Attempt to overwrite if it exists
        if (QFile::exists(targetSqlitePath)) {
            QFile::remove(targetSqlitePath);
            dbCopied = QFile::copy(sqliteDbPath, targetSqlitePath);
        }
    }

    bool qsettingsCopied = QFile::copy(qSettingsPath, targetQSettingsPath);
     if (!qsettingsCopied) {
        if (QFile::exists(targetQSettingsPath)) {
            QFile::remove(targetQSettingsPath);
            qsettingsCopied = QFile::copy(qSettingsPath, targetQSettingsPath);
        }
    }

    if (dbCopied && qsettingsCopied) {
        QMessageBox::information(this, "Export Successful",
                                 QString("Settings successfully exported to:\n%1").arg(backupDir));
    } else {
        QStringList errors;
        if (!dbCopied) errors << QString("Failed to copy database file: %1").arg(QFile(sqliteDbPath).errorString());
        if (!qsettingsCopied) errors << QString("Failed to copy settings file: %1").arg(QFile(qSettingsPath).errorString());
        QMessageBox::critical(this, "Export Failed",
                              QString("Could not export settings.\nErrors:\n%1").arg(errors.join("\n")));
    }
}

void MainWindow::importSettings() {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::warning(this, "Import Settings",
                                 "Importing settings will overwrite your current configuration and close the application.\n"
                                 "You will need to restart it manually for changes to take full effect.\n\n"
                                 "Ensure you have selected a folder previously used for export.\n\n"
                                 "Continue with import?",
                                 QMessageBox::Yes | QMessageBox::Cancel);
    if (reply == QMessageBox::Cancel) {
        return;
    }

    QString backupDir = QFileDialog::getExistingDirectory(this, "Select Folder Containing Backup",
                                                        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    if (backupDir.isEmpty()) {
        return; // User cancelled
    }

    // Define expected source file names (these should match what exportSettings uses)
    QFileInfo currentDbInfo(m_dbManager->databasePath());
    QString sourceSqliteFile = backupDir + "/" + currentDbInfo.fileName();

    QSettings currentQSettings;
    QFileInfo currentQSettingsInfo(currentQSettings.fileName());
    QString sourceQSettingsFile = backupDir + "/" + currentQSettingsInfo.fileName();

    if (!QFile::exists(sourceSqliteFile) || !QFile::exists(sourceQSettingsFile)) {
        QMessageBox::critical(this, "Import Error",
                              "Backup folder does not contain the required settings files (database and .ini/config file).");
        return;
    }

    // Target paths
    QString targetSqliteDbPath = m_dbManager->databasePath();
    QString targetQsettingsPath = currentQSettings.fileName();

    // Close DB connection before overwriting
    m_dbManager->closeDatabase();
    // For QSettings, it's tricky. QSettings writes on destruction or sync().
    // Overwriting the file while the app is running might be problematic.
    // The app will quit anyway.

    bool dbImported = false;
    if (QFile::exists(targetSqliteDbPath)) QFile::remove(targetSqliteDbPath); // Remove old before copy
    dbImported = QFile::copy(sourceSqliteFile, targetSqliteDbPath);

    bool qsettingsImported = false;
    if (QFile::exists(targetQsettingsPath)) QFile::remove(targetQsettingsPath); // Remove old
    qsettingsImported = QFile::copy(sourceQSettingsFile, targetQsettingsPath);


    if (dbImported && qsettingsImported) {
        QMessageBox::information(this, "Import Successful",
                                 "Settings imported successfully.\n"
                                 "The application will now close. Please restart it manually.");
        qApp->quit(); // Quit the application
    } else {
        QStringList errors;
        if (!dbImported) errors << QString("Failed to import database file. Original might be restored or corrupted. Error: %1").arg(QFile(sourceSqliteFile).errorString());
        if (!qsettingsImported) errors << QString("Failed to import settings file. Original might be restored or corrupted. Error: %1").arg(QFile(sourceQSettingsFile).errorString());
        QMessageBox::critical(this, "Import Failed",
                              QString("Could not import all settings.\nErrors:\n%1\n\nIt's recommended to check application data or restore from another backup.").arg(errors.join("\n")));
        // Re-open DB if it was closed, so app can continue somewhat if user doesn't quit.
        // However, state is now mixed. Quitting is safer.
         qApp->quit();
    }
}


// --- Settings Load/Save ---
void MainWindow::handleCurrentTabChanged(int index)
{
    if (index >= 0 && index < m_tabWidget->count()) {
        PageTabContentWidget* tabContent = qobject_cast<PageTabContentWidget*>(m_tabWidget->widget(index));
        if (tabContent) {
            QUuid pageId = tabContent->pageId();
            if (m_pageManager->activePage() == nullptr || (m_pageManager->activePage() && m_pageManager->activePage()->id() != pageId) ) {
                 qDebug() << "Tab changed by user to index:" << index << "Page ID:" << pageId;
                m_pageManager->setActivePageById(pageId);
            }
        }
    } else if (index == -1 && m_pageManager->activePageIndex() != -1) {
        // All tabs closed or no tab selected
        m_pageManager->setActivePageIndex(-1);
    }
}


void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    // The main window is transparent. Child widgets (like tab bar) will have their own backgrounds.
    // If we want a global background effect for the "empty" areas of the overlay, it would be drawn here.
    // For now, let's keep it fully transparent.
    painter.fillRect(rect(), Qt::transparent);

    // Example: Draw a border to see window extents, useful for debugging layout
    // painter.setPen(QColor(0, 255, 0, 100)); // Green, semi-transparent
    // painter.drawRect(rect().adjusted(0, 0, -1, -1));
}

// Mouse events are now largely handled by the QTabWidget and QPushButton.
// If we need to drag the entire window (e.g., from an empty area not covered by controls),
// these methods would need to be reinstated and refined.
/*
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    // Example: Only allow dragging if pressing on an empty area (not on tabs/buttons)
    // QWidget* child = childAt(event->pos());
    // if (!child || child == m_centralWidget) { // Or a specific draggable area widget
    //     if (event->button() == Qt::LeftButton) {
    //         m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
    //         event->accept();
    //     }
    // } else {
    //     QMainWindow::mousePressEvent(event); // Pass to children
    // }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    // if (event->buttons() & Qt::LeftButton && !childAt(event->pos())) { // Check if dragging is active
    //     move(event->globalPosition().toPoint() - m_dragPosition);
    //     event->accept();
    // } else {
    //    QMainWindow::mouseMoveEvent(event);
    // }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    // QMainWindow::mouseReleaseEvent(event);
}
*/

// --- Zone Signal Handlers from PageManager ---

void MainWindow::handleZoneAddedToPage(PageData* page, ZoneData* zoneData)
{
    if (!page || !zoneData) return;

    // Find the PageTabContentWidget for this page
    for (int i = 0; i < m_tabWidget->count(); ++i) {
        PageTabContentWidget* tabContent = qobject_cast<PageTabContentWidget*>(m_tabWidget->widget(i));
        if (tabContent && tabContent->pageId() == page->id()) {
            tabContent->handleZoneAdded(page, zoneData);
            return;
        }
    }
    qWarning() << "MainWindow: Could not find PageTabContentWidget for page" << page->id() << "to add zone" << zoneData->id();
}

void MainWindow::handleZoneRemovedFromPage(PageData* page, QUuid zoneId)
{
    if (!page) return;

    for (int i = 0; i < m_tabWidget->count(); ++i) {
        PageTabContentWidget* tabContent = qobject_cast<PageTabContentWidget*>(m_tabWidget->widget(i));
        if (tabContent && tabContent->pageId() == page->id()) {
            tabContent->handleZoneRemoved(page, zoneId);
            return;
        }
    }
    qWarning() << "MainWindow: Could not find PageTabContentWidget for page" << page->id() << "to remove zone" << zoneId;
}

void MainWindow::handleZoneDataChanged(ZoneData* zone)
{
    if (!zone) return;

    // This signal is less specific about which page it's for,
    // so we might need to iterate all PageTabContentWidgets or PageManager needs to provide page context.
    // For now, let's assume PageManager might emit this for a zone on any page.
    // A better approach: ZoneDataChanged signal from PageManager should include PageData*

    // Find which page this zone belongs to by asking PageManager (if possible) or iterating.
    // For now, we assume the ZoneWidget itself will update if its ZoneData pointer is the same.
    // This is simpler if PageTabContentWidget receives this signal directly for its zones.
    // Let's refine this: The signal will be received by PageTabContentWidget directly if it connects.
    // MainWindow will forward it to the *active* page's content for now, or all.

    for (int i = 0; i < m_tabWidget->count(); ++i) {
        PageTabContentWidget* tabContent = qobject_cast<PageTabContentWidget*>(m_tabWidget->widget(i));
        if (tabContent && tabContent->pageData() && tabContent->pageData()->zoneById(zone->id())) {
            tabContent->handleZoneDataChanged(zone);
            // If a zone could be on multiple pages (not current design), this would need to call all relevant.
            // return; // Found and handled
        }
    }
     //qDebug() << "MainWindow: ZoneDataChanged for zone" << zone->id();
}


// --- UI Action Implementations ---

void MainWindow::addZoneToCurrentPage()
{
    PageData* currentPage = m_pageManager->activePage();
    if (!currentPage) {
        qDebug() << "No active page to add zone to.";
        return;
    }

    // Create a new zone with some default properties
    // Position and size should ideally be somewhat intelligent, e.g., not overlapping existing ones,
    // or at a default spot. For now, a fixed default.
    // Parent widget for ZoneWidget is PageTabContentWidget. Geometry is relative to it.
    QRectF defaultGeometry(50, 50, 200, 150); // x, y, width, height
    QColor defaultColor = QColor(Qt::darkGray).darker(120); // Semi-dark, slightly transparent if desired later
    defaultColor.setAlpha(180); // Example alpha

    QString defaultTitle = QString("New Zone %1").arg(currentPage->zones().count() + 1);

    m_pageManager->addZoneToActivePage(defaultTitle, defaultGeometry, defaultColor);
    // The handleZoneAddedToPage slot will then trigger the UI update in PageTabContentWidget.
    qDebug() << "Add Zone button clicked for page:" << currentPage->name();
}

// --- Settings Load/Save ---
void MainWindow::loadSettings()
{
    // Load theme preference first
    ThemeManager::Theme preferredTheme = ThemeManager::loadThemePreference();
    ThemeManager::setCurrentTheme(preferredTheme);
    // applyCurrentTheme() will be called separately after this in constructor,
    // or setCurrentTheme itself can call applyTheme.
    // For clarity, ensure applyCurrentTheme updates menu checks.

    if (m_dbManager->openDatabase()) {
        if (!m_dbManager->loadPages(m_pageManager)) {
            qWarning() << "MainWindow: Failed to load pages from database. Starting with a default page.";
            m_pageManager->clearAllPages();
            m_pageManager->addPage("Default Page");
        } else {
            if (m_pageManager->pageCount() == 0) {
                qDebug() << "MainWindow: Database was empty or no pages loaded. Creating default page.";
                m_pageManager->addPage("Default Page");
            } else {
                qDebug() << "MainWindow: Pages loaded successfully from database.";
            }
        }
    } else {
        qWarning() << "MainWindow: Could not open database. Starting with a default page.";
        m_pageManager->clearAllPages();
        m_pageManager->addPage("Default Page");
    }

    // UI update for tabs after loading pages
    if(m_pageManager->pageCount() > 0 && m_tabWidget->count() > 0) {
        int activeIdx = m_pageManager->activePageIndex();
        if (activeIdx != -1 && activeIdx < m_tabWidget->count()) {
             m_tabWidget->setCurrentIndex(activeIdx);
        } else if (m_tabWidget->count() > 0) {
            // If active index from manager is invalid, or -1 but we have tabs
            m_tabWidget->setCurrentIndex(0);
            m_pageManager->setActivePageIndex(0); // Sync manager if it was -1
        }
    } else if (m_pageManager->pageCount() == 0 && m_tabWidget->count() > 0) {
        // This case might happen if default page was added but then load cleared it
        // and then another default page was added by loadSettings itself.
        // Essentially, ensure tabwidget and pagemanager are in sync.
        if (m_tabWidget->count() == 1 && m_pageManager->pageCount() ==1) {
             m_tabWidget->setCurrentIndex(0);
             m_pageManager->setActivePageIndex(0);
        }
    }

    }

    // Load hosted widgets from QSettings
    QSettings settings;
    settings.beginGroup("HostedWidgets");
    QStringList widgetKeys = settings.childGroups(); // Get keys like "FloatingClockHost", "MainToolbar"
    settings.endGroup(); // Important to end group before iterating its children

    for (const QString& widgetKey : widgetKeys) {
        settings.beginGroup("HostedWidgets/" + widgetKey); // Start group for specific widget

        // QString type = settings.value("type").toString(); // Could use this if saved
        QRect geometry = settings.value("geometry").toRect();
        bool visible = settings.value("visible", true).toBool();

        WidgetHostWindow* host = nullptr;

        // Recreate based on objectName (widgetKey)
        if (widgetKey == "FloatingClockHost") {
            // Check if one with this object name already exists (e.g. from showNewFloatingClock if default)
            // This logic can get complex if we allow multiple instances vs. unique named instances.
            // For now, assume we load what's in settings. If menu creates another, it's a new one.
            ClockWidget* clock = new ClockWidget();
            host = new WidgetHostWindow();
            host->setContentWidget(clock);
            host->setWindowTitle("Clock");
            host->setObjectName("FloatingClockHost"); // Set object name for future saves
        } else if (widgetKey == "MainToolbar") {
            DraggableToolbar* toolbar = new DraggableToolbar(DraggableToolbar::Orientation::Horizontal);
            // TODO: Persist/restore toolbar content if necessary. For now, default content.
            ClockWidget* clock = new ClockWidget(toolbar);
            toolbar->addWidget(clock);
            toolbar->setWindowTitle("Toolbar");
            host = toolbar;
            host->setObjectName("MainToolbar");
        } else if (widgetKey == "QuickAccessPanelHost") {
            QuickAccessPanel* panel = new QuickAccessPanel();
            host = new WidgetHostWindow();
            host->setContentWidget(panel);
            host->setWindowTitle("Quick Access");
            host->setObjectName("QuickAccessPanelHost");
        } else if (widgetKey == "TodoWidgetHost") {
            TodoWidget* todo = new TodoWidget();
            host = new WidgetHostWindow();
            host->setContentWidget(todo);
            host->setWindowTitle("To-Do List");
            host->setObjectName("TodoWidgetHost");
        }
        // Add more 'else if' blocks for other persistable widget types

        if (host) {
            host->setAttribute(Qt::WA_DeleteOnClose);
            connect(host, &QObject::destroyed, this, &MainWindow::handleHostedWidgetDestroyed);
            m_hostedWidgets.append(host);

            // Validate and set geometry
            bool geometryValid = false;
            if (!geometry.isNull()) {
                for (QScreen* screen : QGuiApplication::screens()) {
                    if (screen->geometry().intersects(geometry)) { // Simple intersection check
                        geometryValid = true;
                        break;
                    }
                }
            }

            if (geometryValid) {
                host->setGeometry(geometry);
            } else {
                if (!geometry.isNull()) {
                     qWarning() << "Hosted widget" << widgetKey << "saved geometry" << geometry << "is off-screen. Resetting to default position.";
                }
                // Default position on primary screen (adjust as needed, maybe cascade)
                QScreen* primaryScreen = QGuiApplication::primaryScreen();
                if (primaryScreen) {
                    QRect defaultHostGeom = host->geometry(); // Get its sizeHint based geometry
                    defaultHostGeom.moveCenter(primaryScreen->availableGeometry().center());
                    // Ensure it's fully on screen
                    if (defaultHostGeom.left() < primaryScreen->availableGeometry().left())
                        defaultHostGeom.moveLeft(primaryScreen->availableGeometry().left());
                    if (defaultHostGeom.top() < primaryScreen->availableGeometry().top())
                        defaultHostGeom.moveTop(primaryScreen->availableGeometry().top());
                    if (defaultHostGeom.right() > primaryScreen->availableGeometry().right())
                        defaultHostGeom.moveRight(primaryScreen->availableGeometry().right());
                    if (defaultHostGeom.bottom() > primaryScreen->availableGeometry().bottom())
                        defaultHostGeom.moveBottom(primaryScreen->availableGeometry().bottom());
                    host->setGeometry(defaultHostGeom);
                } else { // Fallback if no primary screen (highly unlikely)
                     host->adjustSize(); // Let it take its sizeHint
                }
            }

            if (visible) {
                host->show();
            } else {
                host->hide();
            }
            qDebug() << "Loaded hosted widget:" << widgetKey << "Visible:" << visible << "Geo:" << host->geometry();
        } else {
            qWarning() << "Unknown or unhandled hosted widget key in settings:" << widgetKey;
        }
        settings.endGroup(); // End specific widget group
    }
    if (!widgetKeys.isEmpty()) {
        qDebug() << "Finished processing" << widgetKeys.count() << "saved hosted widget configurations.";
    }
}


// --- Floating Widget/Toolbar Creation & Management ---
void MainWindow::showNewFloatingClock() {
    // Check if a clock already exists to prevent multiple (or manage them)
    // For now, allow multiple for testing.
    ClockWidget* clock = new ClockWidget();
    WidgetHostWindow* host = new WidgetHostWindow();
    host->setContentWidget(clock);
    host->setWindowTitle("Clock");
    host->setObjectName("FloatingClockHost");
    host->setAttribute(Qt::WA_DeleteOnClose);
    connect(host, &QObject::destroyed, this, &MainWindow::handleHostedWidgetDestroyed);

    // Try to load last geometry for "FloatingClockHost" if it was a unique instance
    // Or, position new ones intelligently.
    QSettings settings;
    host->setGeometry(settings.value("HostedWidgets/FloatingClockHost/geometry", QRect(100, 100, 150, 70)).toRect());


    m_hostedWidgets.append(host);
    host->show();
    qDebug() << "Showing new floating clock:" << host->objectName();
}

void MainWindow::showNewToolbar() {
    // For now, only one main toolbar is created/managed.
    // Could be extended to allow multiple named toolbars.
    for(WidgetHostWindow* whw : m_hostedWidgets) {
        if(whw->objectName() == "MainToolbar") {
            whw->show();
            whw->raise();
            return; // Toolbar already exists, just show it.
        }
    }

    DraggableToolbar* toolbar = new DraggableToolbar(DraggableToolbar::Orientation::Horizontal);
    toolbar->setWindowTitle("Toolbar");
    toolbar->setObjectName("MainToolbar");
    toolbar->setAttribute(Qt::WA_DeleteOnClose);
    connect(toolbar, &QObject::destroyed, this, &MainWindow::handleHostedWidgetDestroyed);

    ClockWidget* clock = new ClockWidget(toolbar);
    toolbar->addWidget(clock);
    toolbar->addSeparator();
    // Add more default items if needed

    QSettings settings;
    toolbar->setGeometry(settings.value("HostedWidgets/MainToolbar/geometry", QRect(50, 50, 300, 60)).toRect());

    m_hostedWidgets.append(toolbar);
    toolbar->show();
    qDebug() << "Showing new toolbar:" << toolbar->objectName();
}

void MainWindow::showQuickAccessPanel() {
    for(WidgetHostWindow* whw : m_hostedWidgets) {
        if(whw->objectName() == "QuickAccessPanelHost") {
            whw->show();
            whw->raise();
            return; // Panel already exists
        }
    }

    QuickAccessPanel* panel = new QuickAccessPanel();
    WidgetHostWindow* host = new WidgetHostWindow();
    host->setContentWidget(panel);
    host->setWindowTitle("Quick Access");
    host->setObjectName("QuickAccessPanelHost");
    host->setAttribute(Qt::WA_DeleteOnClose);
    connect(host, &QObject::destroyed, this, &MainWindow::handleHostedWidgetDestroyed);

    QSettings settings;
    host->setGeometry(settings.value("HostedWidgets/QuickAccessPanelHost/geometry", QRect(30, 100, 200, 400)).toRect());
    // Adjust default size as needed

    m_hostedWidgets.append(host);
    host->show();
    qDebug() << "Showing new Quick Access Panel:" << host->objectName();
}


void MainWindow::handleHostedWidgetDestroyed(QObject* obj) {
    WidgetHostWindow* hostWindow = qobject_cast<WidgetHostWindow*>(obj);
    if (hostWindow) {
        bool removed = m_hostedWidgets.removeAll(hostWindow);
        if (removed) {
            qDebug() << "Hosted widget" << hostWindow->objectName() << "destroyed and removed from tracking list.";
        } else {
            qDebug() << "Hosted widget" << hostWindow->objectName() << "destroyed, but was not in tracking list (or already removed).";
        }
    }
}


// --- Page Context Menu & Property Handlers ---
void MainWindow::showPageContextMenu(const QPoint& point) {
    QTabBar* tabBar = m_tabWidget->tabBar();
    int tabIndex = tabBar->tabAt(point);
    if (tabIndex == -1) return;

    PageTabContentWidget* tabContent = qobject_cast<PageTabContentWidget*>(m_tabWidget->widget(tabIndex));
    if (!tabContent || !tabContent->pageData()) return;
    PageData* pageData = tabContent->pageData();

    QMenu contextMenu(this);
    QAction* setWallpaperAction = contextMenu.addAction("Set Page Wallpaper...");
    connect(setWallpaperAction, &QAction::triggered, this, [this, pageData](){ setPageWallpaper(pageData); });

    if (!pageData->wallpaperPath().isEmpty()) {
        QAction* clearWallpaperAction = contextMenu.addAction("Clear Page Wallpaper");
        connect(clearWallpaperAction, &QAction::triggered, this, [this, pageData](){ clearPageWallpaper(pageData); });
    }

    contextMenu.addSeparator();
    QAction* setOverlayAction = contextMenu.addAction("Set Page Overlay Color...");
    connect(setOverlayAction, &QAction::triggered, this, [this, pageData](){ setPageOverlayColor(pageData); });

    if (pageData->overlayColor().alpha() > 0) {
        QAction* clearOverlayAction = contextMenu.addAction("Clear Page Overlay Color");
        connect(clearOverlayAction, &QAction::triggered, this, [this, pageData](){ clearPageOverlayColor(pageData); });
    }

    contextMenu.exec(tabBar->mapToGlobal(point));
}

void MainWindow::setPageWallpaper(PageData* pageData) {
    if (!pageData || !m_pageManager) return;
    QString filePath = QFileDialog::getOpenFileName(this, "Select Wallpaper Image",
                                                    QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
                                                    "Images (*.png *.jpg *.jpeg *.bmp)");
    if (!filePath.isEmpty()) {
        pageData->setWallpaperPath(filePath);
        m_pageManager->notifyPagePropertiesChanged(pageData);
    }
}

void MainWindow::clearPageWallpaper(PageData* pageData) {
    if (!pageData || !m_pageManager) return;
    if (!pageData->wallpaperPath().isEmpty()) {
        pageData->setWallpaperPath(QString());
        m_pageManager->notifyPagePropertiesChanged(pageData);
    }
}

void MainWindow::setPageOverlayColor(PageData* pageData) {
    if (!pageData || !m_pageManager) return;
    QColor newColor = QColorDialog::getColor(pageData->overlayColor(), this, "Select Overlay Color", QColorDialog::ShowAlphaChannel);
    if (newColor.isValid()) {
        pageData->setOverlayColor(newColor);
        m_pageManager->notifyPagePropertiesChanged(pageData);
    }
}

void MainWindow::clearPageOverlayColor(PageData* pageData) {
    if (!pageData || !m_pageManager) return;
    if (pageData->overlayColor().alpha() > 0) {
        pageData->setOverlayColor(Qt::transparent);
        m_pageManager->notifyPagePropertiesChanged(pageData);
    }
}

void MainWindow::handlePagePropertiesChanged(PageData* pageData) {
    if (!pageData || !m_tabWidget) return;
    for (int i = 0; i < m_tabWidget->count(); ++i) {
        PageTabContentWidget* tabContent = qobject_cast<PageTabContentWidget*>(m_tabWidget->widget(i));
        if (tabContent && tabContent->pageId() == pageData->id()) {
            tabContent->update(); // Trigger a repaint of the page content area
            return;
        }
    }
}


// --- Settings Load/Save ---
void MainWindow::saveSettings()
{
    // Save Page/Zone/Icon structure to SQLite
    if (m_dbManager->openDatabase()) {
        if (!m_dbManager->savePages(m_pageManager->pages())) {
            qWarning() << "MainWindow: Failed to save page structure to database.";
        } else {
            qDebug() << "MainWindow: Page structure saved successfully to database.";
        }
        m_dbManager->closeDatabase();
    } else {
        qWarning() << "MainWindow: Could not open database to save page structure.";
    }

    // Save floating widget/toolbar states to QSettings
    QSettings settings;
    // For specific, named widgets like "FloatingClockHost" or "MainToolbar"
    // we can save them directly by their objectName as a group key.
    for (WidgetHostWindow* host : m_hostedWidgets) {
        if (!host || host->objectName().isEmpty()) continue;
        settings.beginGroup("HostedWidgets/" + host->objectName());
        settings.setValue("type", host->metaObject()->className()); // Store actual class type
        settings.setValue("geometry", host->geometry());
        settings.setValue("visible", host->isVisible());
        // For toolbars, might also save orientation
        if (DraggableToolbar* tb = qobject_cast<DraggableToolbar*>(host)) {
            // settings.setValue("orientation", tb->orientation() == DraggableToolbar::Orientation::Horizontal ? "Horizontal" : "Vertical");
        }
        settings.endGroup();
    }
    qDebug() << "Hosted widget states saved to QSettings using object names.";
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    qDebug() << "MainWindow closeEvent: Saving settings...";
    saveSettings();
    QMainWindow::closeEvent(event); // Accept the close event
}


// --- New slots for Page Management Enhancements ---

void MainWindow::handlePageNameChanged(PageData* page) {
    if (!page || !m_tabWidget) return;

    for (int i = 0; i < m_tabWidget->count(); ++i) {
        PageTabContentWidget* tabContent = qobject_cast<PageTabContentWidget*>(m_tabWidget->widget(i));
        if (tabContent && tabContent->pageId() == page->id()) {
            m_tabWidget->setTabText(i, page->name());
            qDebug() << "Tab text updated for page ID" << page->id() << "to" << page->name();
            return;
        }
    }
    qWarning() << "Could not find tab to update name for page ID" << page->id();
}

void MainWindow::handleTabMoved(int fromIndex, int toIndex) {
    if (!m_pageManager) return;
    qDebug() << "Tab moved in UI from" << fromIndex << "to" << toIndex;
    m_pageManager->movePage(fromIndex, toIndex);
    // Persistence of order is handled by PageManager saving pages in their current list order.
}

void MainWindow::handleScreenGeometryChanged(const QRect& newGeometry) {
    qDebug() << "Primary screen geometry changed to:" << newGeometry;
    this->setGeometry(newGeometry); // Resize main window to new screen geometry

    // Optional: Re-validate positions of hosted widgets if they should be
    // constrained to the primary screen or readjusted based on its new size.
    // For now, their own saved positions (validated at load) will be used unless
    // they are moved by the user. If a widget was on this primary screen and the screen
    // shrinks, it might become partially off-screen.
    // A simple check:
    for (WidgetHostWindow* host : m_hostedWidgets) {
        if (host && host->isVisible()) {
            QScreen* hostScreen = QGuiApplication::screenAt(host->geometry().center());
            if (hostScreen == QGuiApplication::primaryScreen()) { // Only adjust if it was on primary
                QRect currentHostGeo = host->geometry();
                // If it's now outside available geometry, move it in (simple example)
                if (!QGuiApplication::primaryScreen()->availableGeometry().intersects(currentHostGeo)) {
                    qDebug() << "Hosted widget" << host->objectName() << "is now off primary screen due to resize. Repositioning.";
                    // Simplified reposition - might need better logic (e.g. preserve offset from an edge)
                    currentHostGeo.moveTo(QGuiApplication::primaryScreen()->availableGeometry().topLeft() + QPoint(20,20));
                    host->setGeometry(currentHostGeo);
                }
            }
        }
    }
}

void MainWindow::handleTabDoubleClicked(int index) {
    if (index < 0 || index >= m_tabWidget->count() || !m_pageManager) return;

    PageTabContentWidget* tabContent = qobject_cast<PageTabContentWidget*>(m_tabWidget->widget(index));
    if (!tabContent) return;

    QUuid pageId = tabContent->pageId();
    PageData* pageData = m_pageManager->pageById(pageId);
    if (!pageData) {
        qWarning() << "No PageData found for tab double-clicked at index" << index;
        return;
    }

    bool ok;
    QString currentName = pageData->name();
    QString newName = QInputDialog::getText(this, "Rename Page", "Enter new page name:", QLineEdit::Normal, currentName, &ok);

    if (ok && !newName.isEmpty() && newName != currentName) {
        if (m_pageManager->renamePage(pageId, newName)) {
            // Name change in tab widget will be handled by handlePageNameChanged slot
            qDebug() << "Page rename requested for ID" << pageId << "to" << newName;
        } else {
            QMessageBox::warning(this, "Rename Failed", "Could not rename the page.");
        }
    }
}
