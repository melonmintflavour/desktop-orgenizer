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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_pageManager(new PageManager(this)),
      m_dbManager(new DatabaseManager("DesktopOverlay.sqlite", this)) // Create DB Manager
{
    setupUI();
    setupPageControls();
    loadSettings(); // Load settings after UI setup

    // Connect PageManager signals to MainWindow slots
    connect(m_pageManager, &PageManager::pageAdded, this, &MainWindow::onPageAddedToManager);
    connect(m_pageManager, &PageManager::pageRemoved, this, &MainWindow::onPageRemovedFromManager);
    connect(m_pageManager, &PageManager::activePageChanged, this, &MainWindow::onActivePageChanged);
    // connect(m_pageManager, &PageManager::pageNameChanged, this, &MainWindow::onPageNameChangedInManager); // TODO
    connect(m_pageManager, &PageManager::zoneAddedToPage, this, &MainWindow::handleZoneAddedToPage);
    connect(m_pageManager, &PageManager::zoneRemovedFromPage, this, &MainWindow::handleZoneRemovedFromPage);
    connect(m_pageManager, &PageManager::zoneDataChanged, this, &MainWindow::handleZoneDataChanged);

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


    m_controlsLayout->addWidget(m_tabWidget, 1); // Tab widget takes most space
    m_controlsLayout->addWidget(m_addPageButton);
    m_controlsLayout->addWidget(m_addZoneButton); // Add the new button

    m_pageControlsWidget->setLayout(m_controlsLayout);

    // Add the controls widget to the top of the main layout
    m_mainLayout->addWidget(m_pageControlsWidget);

    // Add a stretch to push content below tabs (actual page content will go here)
    m_mainLayout->addStretch(1);

    // Style the tab bar and button a bit for visibility on transparent background
    // This is basic, more advanced styling can be done with stylesheets
    m_pageControlsWidget->setStyleSheet(
        "QWidget { background-color: rgba(0, 0, 0, 120); border-radius: 5px; }"
        "QTabWidget::pane { border: none; background: transparent; }"
        "QTabBar::tab { background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #555555, stop: 1 #333333);"
        "              color: white; padding: 8px; border-top-left-radius: 4px; border-top-right-radius: 4px; min-width: 100px; }"
        "QTabBar::tab:selected { background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #777777, stop: 1 #555555); }"
        "QTabBar::tab:hover { background: #666666; }"
        "QPushButton { background-color: #555555; color: white; border: 1px solid #777777; border-radius: 15px; font-size: 16pt; }"
        "QPushButton:hover { background-color: #666666; }"
        "QPushButton:pressed { background-color: #444444; }"
    );
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

void MainWindow::handleTabCloseRequested(int index)
{
    if (index < 0 || index >= m_tabWidget->count()) return;

    PageTabContentWidget* tabContent = qobject_cast<PageTabContentWidget*>(m_tabWidget->widget(index));
    if (tabContent) {
        QUuid pageId = tabContent->pageId();
        qDebug() << "Tab close requested for page ID:" << pageId << "at tab index" << index;
        m_pageManager->removePageById(pageId); // This will trigger onPageRemovedFromManager
    }
}

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
    if (m_dbManager->openDatabase()) {
        if (!m_dbManager->loadPages(m_pageManager)) {
            qWarning() << "MainWindow: Failed to load pages from database. Starting with a default page.";
            // Ensure any partially loaded state from a failed load is cleared before adding default
            m_pageManager->clearAllPages();
            m_pageManager->addPage("Default Page"); // Create a default page if loading fails
        } else {
            if (m_pageManager->pageCount() == 0) {
                qDebug() << "MainWindow: Database was empty or no pages loaded. Creating default page.";
                m_pageManager->addPage("Default Page"); // Create a default page if DB is empty
            } else {
                qDebug() << "MainWindow: Pages loaded successfully from database.";
            }
        }
        // m_dbManager->closeDatabase(); // Keep open or close? For now, keep open until app close.
    } else {
        qWarning() << "MainWindow: Could not open database. Starting with a default page.";
        m_pageManager->clearAllPages();
        m_pageManager->addPage("Default Page"); // Create a default page if DB can't be opened
    }

    // Ensure UI reflects the loaded state (especially if PageManager signals weren't fully connected yet)
    // This might involve refreshing the tab widget if addLoadedPage didn't trigger UI updates.
    // For now, onPageAddedToManager should handle creating tabs for loaded pages.
    // If active page needs to be set after loading:
    if(m_pageManager->pageCount() > 0 && m_tabWidget->count() > 0) {
        if (m_pageManager->activePageIndex() != -1 && m_pageManager->activePageIndex() < m_tabWidget->count()) {
             m_tabWidget->setCurrentIndex(m_pageManager->activePageIndex());
        } else if (m_tabWidget->count() > 0) {
            m_tabWidget->setCurrentIndex(0); // Fallback to first tab
            m_pageManager->setActivePageIndex(0);
        }
    }
}

void MainWindow::saveSettings()
{
    if (m_dbManager->openDatabase()) { // Ensure it's open (might have been closed)
        if (!m_dbManager->savePages(m_pageManager->pages())) {
            qWarning() << "MainWindow: Failed to save pages to database.";
            // Optionally: notify user here with a simple message box
        } else {
            qDebug() << "MainWindow: Pages saved successfully to database.";
        }
        m_dbManager->closeDatabase(); // Close after saving
    } else {
        qWarning() << "MainWindow: Could not open database to save settings.";
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    qDebug() << "MainWindow closeEvent: Saving settings...";
    saveSettings();
    QMainWindow::closeEvent(event); // Accept the close event
}
