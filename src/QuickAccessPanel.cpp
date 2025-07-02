#include "QuickAccessPanel.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>
#include <QStyle>
#include <QSettings>
#include <QFileInfo>
#include <QToolButton>
#include <QLabel>
#include <QScrollArea>
#include <QMenu>
#include <QFileIconProvider>


const QString PINNED_ITEMS_KEY = "QuickAccessPanel/pinnedItems";

QuickAccessPanel::QuickAccessPanel(QWidget *parent)
    : QWidget(parent), m_iconProvider(new QFileIconProvider)
{
    setObjectName("QuickAccessPanel");
    setAcceptDrops(true); // Enable D&D for the whole panel
    setupUI();
    loadPinnedItems();
}

QuickAccessPanel::~QuickAccessPanel()
{
    savePinnedItems(); // Save on destruction (or when panel is hidden/app closes)
    delete m_iconProvider;
    qDebug() << "QuickAccessPanel destroyed";
}

void QuickAccessPanel::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(5, 5, 5, 5);
    m_mainLayout->setSpacing(8); // Increased spacing

    // --- Common Folders Section ---
    QLabel* commonFoldersLabel = new QLabel("Common Folders", this);
    commonFoldersLabel->setObjectName("QuickAccessSectionHeader");
    m_mainLayout->addWidget(commonFoldersLabel);

    QVBoxLayout* commonFoldersLayout = new QVBoxLayout(); // Sub-layout
    commonFoldersLayout->setSpacing(2);

    QList<QPair<QString, QStandardPaths::StandardLocation>> commonFolders;
    commonFolders.append({"Documents", QStandardPaths::DocumentsLocation});
    commonFolders.append({"Downloads", QStandardPaths::DownloadLocation});
    commonFolders.append({"Pictures", QStandardPaths::PicturesLocation});
    // Add more if desired (Music, Videos, Desktop)

    for (const auto& folderPair : commonFolders) {
        QString path = QStandardPaths::writableLocation(folderPair.second);
        if (!path.isEmpty()) {
            addFolderShortcut(folderPair.first, path, commonFoldersLayout);
        }
    }
    m_mainLayout->addLayout(commonFoldersLayout);
    m_mainLayout->addSpacing(10);

    // --- Pinned Items Section ---
    QLabel* pinnedItemsLabel = new QLabel("Pinned Items", this);
    pinnedItemsLabel->setObjectName("QuickAccessSectionHeader");
    m_mainLayout->addWidget(pinnedItemsLabel);

    m_pinnedItemsScrollArea = new QScrollArea(this);
    m_pinnedItemsScrollArea->setWidgetResizable(true);
    m_pinnedItemsScrollArea->setFrameShape(QFrame::NoFrame); // No border for scroll area itself
    m_pinnedItemsScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_pinnedItemsContainer = new QWidget(m_pinnedItemsScrollArea);
    m_pinnedItemsLayout = new QVBoxLayout(m_pinnedItemsContainer);
    m_pinnedItemsLayout->setContentsMargins(0,0,0,0);
    m_pinnedItemsLayout->setSpacing(2);
    m_pinnedItemsLayout->setAlignment(Qt::AlignTop);
    m_pinnedItemsContainer->setLayout(m_pinnedItemsLayout);

    m_pinnedItemsScrollArea->setWidget(m_pinnedItemsContainer);
    m_mainLayout->addWidget(m_pinnedItemsScrollArea, 1); // Scroll area takes remaining space

    setLayout(m_mainLayout);
    setMinimumWidth(200);
    // Apply some basic styling for section headers, can be moved to theme later
    setStyleSheet(R"(
        QLabel#QuickAccessSectionHeader {
            font-weight: bold;
            margin-top: 5px;
            margin-bottom: 3px;
            /* color: from theme */
        }
        QToolButton { /* Styling for pinned items */
            text-align: left;
            padding: 5px;
            border: none; /* Flat look */
            background-color: transparent;
            /* icon-size: 16px; */ /* Handled by button's setIconSize */
        }
        QToolButton:hover {
            background-color: rgba(0,0,0,0.1); /* Theme dependent */
        }
    )");
}

void QuickAccessPanel::addFolderShortcut(const QString& name, const QString& path, QVBoxLayout* layout)
{
    QPushButton* button = new QPushButton(this);
    button->setText(name);
    button->setToolTip(path);
    button->setFocusPolicy(Qt::NoFocus);
    button->setFixedHeight(28);
    button->setStyleSheet("QPushButton { text-align: left; padding-left: 8px; border: none; background-color: transparent; }"
                          "QPushButton:hover { background-color: rgba(0,0,0,0.05); }");

    QIcon icon = m_iconProvider->icon(QFileInfo(path));
    if(icon.isNull()) icon = style()->standardIcon(QStyle::SP_DirIcon);
    button->setIcon(icon);
    button->setIconSize(QSize(16,16));

    connect(button, &QPushButton::clicked, this, &QuickAccessPanel::onFolderButtonClicked);
    m_folderButtonToPathMap.insert(button, path);
    layout->addWidget(button);
}

void QuickAccessPanel::onFolderButtonClicked()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (button && m_folderButtonToPathMap.contains(button)) {
        QString path = m_folderButtonToPathMap.value(button);
        if (!QDesktopServices::openUrl(QUrl::fromLocalFile(path))) {
            qWarning() << "Failed to open folder:" << path;
        }
    }
}

void QuickAccessPanel::loadPinnedItems() {
    QSettings settings;
    m_pinnedItemPaths = settings.value(PINNED_ITEMS_KEY).toStringList();

    // Clear existing pinned item buttons first
    QLayoutItem* item;
    while ((item = m_pinnedItemsLayout->takeAt(0)) != nullptr) {
        delete item->widget(); // Delete the button widget
        delete item;
    }

    for (const QString& path : m_pinnedItemPaths) {
        createPinnedItemButton(path);
    }
}

void QuickAccessPanel::savePinnedItems() {
    QSettings settings;
    settings.setValue(PINNED_ITEMS_KEY, m_pinnedItemPaths);
}

void QuickAccessPanel::addPinnedItem(const QString& path, bool fromLoad) {
    if (path.isEmpty() || m_pinnedItemPaths.contains(path)) {
        return; // Already pinned or empty path
    }
    m_pinnedItemPaths.append(path);
    createPinnedItemButton(path);
    if (!fromLoad) {
        savePinnedItems(); // Save immediately if not loading
    }
}

void QuickAccessPanel::createPinnedItemButton(const QString& path) {
    QFileInfo fileInfo(path);
    QToolButton* button = new QToolButton(m_pinnedItemsContainer);
    button->setText(fileInfo.fileName().isEmpty() ? path : fileInfo.fileName()); // Show full path if no filename
    button->setToolTip(path);
    button->setIcon(m_iconProvider->icon(fileInfo));
    button->setIconSize(QSize(16, 16));
    button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    button->setFixedHeight(28);
    button->setFocusPolicy(Qt::NoFocus);
    button->setProperty("filePath", path); // Store path for context menu

    button->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(button, &QToolButton::customContextMenuRequested, this, [this, button, path](const QPoint &pos){
        QMenu contextMenu(this);
        QAction *openAction = contextMenu.addAction("Open");
        connect(openAction, &QAction::triggered, this, &QuickAccessPanel::onPinnedItemClicked);
        openAction->setData(path); // Store path in action

        QAction *unpinAction = contextMenu.addAction("Unpin");
        connect(unpinAction, &QAction::triggered, this, &QuickAccessPanel::unpinItem);
        unpinAction->setData(path); // Store path in action

        contextMenu.exec(button->mapToGlobal(pos));
    });

    connect(button, &QToolButton::clicked, this, &QuickAccessPanel::onPinnedItemClicked);
    m_pinnedItemsLayout->addWidget(button);
}

QWidget* QuickAccessPanel::findPinnedButtonByPath(const QString& path) {
    for (int i = 0; i < m_pinnedItemsLayout->count(); ++i) {
        QWidget* widget = m_pinnedItemsLayout->itemAt(i)->widget();
        if (widget && widget->property("filePath") == path) {
            return widget;
        }
    }
    return nullptr;
}

void QuickAccessPanel::onPinnedItemClicked() {
    QToolButton* button = qobject_cast<QToolButton*>(sender());
    QString path;
    if (button) { // Clicked directly
        path = button->property("filePath").toString();
    } else { // From context menu action
        QAction* action = qobject_cast<QAction*>(sender());
        if (action) path = action->data().toString();
    }

    if (!path.isEmpty()) {
        if (!QDesktopServices::openUrl(QUrl::fromLocalFile(path))) {
            qWarning() << "Failed to open pinned item:" << path;
        }
    }
}

void QuickAccessPanel::unpinItem() {
    QAction* action = qobject_cast<QAction*>(sender());
    if (action) {
        QString path = action->data().toString();
        if (m_pinnedItemPaths.removeAll(path) > 0) {
            QWidget* buttonWidget = findPinnedButtonByPath(path);
            if (buttonWidget) {
                m_pinnedItemsLayout->removeWidget(buttonWidget);
                buttonWidget->deleteLater();
            }
            savePinnedItems();
        }
    }
}

// --- Drag and Drop ---
void QuickAccessPanel::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls()) {
        // Check if dropping over the pinned items area (optional, or allow drop anywhere on panel)
        // For now, accept anywhere on the panel
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void QuickAccessPanel::dropEvent(QDropEvent *event) {
    const QMimeData* mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QList<QUrl> urls = mimeData->urls();
        int countAdded = 0;
        for (const QUrl& url : urls) {
            if (url.isLocalFile()) {
                QString filePath = url.toLocalFile();
                if (!m_pinnedItemPaths.contains(filePath)) { // Avoid duplicates
                    addPinnedItem(filePath);
                    countAdded++;
                }
            }
        }
        if (countAdded > 0) {
             qDebug() << "Added" << countAdded << "items via drag and drop.";
        }
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}
