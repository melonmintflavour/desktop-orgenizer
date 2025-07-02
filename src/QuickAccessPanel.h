#ifndef QUICKACCESSPANEL_H
#define QUICKACCESSPANEL_H

#include <QWidget>
#include <QMap> // For storing folder paths with buttons

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>

class QPushButton;
class QToolButton; // For pinned items
class QVBoxLayout;
class QScrollArea; // To make pinned items scrollable
class QLabel;      // For section headers
#include <QFileIconProvider> // To get system icons (member variable)

class QuickAccessPanel : public QWidget
{
    Q_OBJECT

public:
    explicit QuickAccessPanel(QWidget *parent = nullptr);
    ~QuickAccessPanel() override;

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    // dragMoveEvent might not be needed if dragEnterEvent accepts appropriately

private slots:
    void onFolderButtonClicked();
    void onPinnedItemClicked();
    void unpinItem(); // Slot for context menu action

private:
    void setupUI();
    void addFolderShortcut(const QString& name, const QString& path, QVBoxLayout* layout);
    void loadPinnedItems();
    void savePinnedItems();
    void addPinnedItem(const QString& path, bool fromLoad = false);
    void createPinnedItemButton(const QString& path);
    QWidget* findPinnedButtonByPath(const QString& path);


    QVBoxLayout* m_mainLayout;
    QMap<QWidget*, QString> m_folderButtonToPathMap;

    // Pinned items UI
    QVBoxLayout* m_pinnedItemsLayout; // Layout for pinned item buttons
    QWidget* m_pinnedItemsContainer;  // Widget holding m_pinnedItemsLayout, put into scrollArea
    QScrollArea* m_pinnedItemsScrollArea;
    QStringList m_pinnedItemPaths;     // List of paths for pinned items
    QFileIconProvider* m_iconProvider; // For fetching system icons
};

#endif // QUICKACCESSPANEL_H
