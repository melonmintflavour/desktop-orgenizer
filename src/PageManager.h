#ifndef PAGEMANAGER_H
#define PAGEMANAGER_H

#include <QObject>
#include <QList>
#include <QString>
#include "PageData.h"
#include "ZoneData.h" // Include for signal/slot parameters

class PageManager : public QObject
{
    Q_OBJECT

public:
    explicit PageManager(QObject *parent = nullptr);

    const QList<PageData*>& pages() const;
    PageData* page(int index) const;
    PageData* pageById(const QUuid& id) const;
    int pageCount() const;

    PageData* addPage(const QString& name);
    bool removePage(int index);
    bool removePageById(const QUuid& id);

    int activePageIndex() const;
    PageData* activePage() const;

public slots:
    void setActivePageIndex(int index);
    void setActivePageById(const QUuid& id);

signals:
    void pageAdded(PageData* page, int index);
    void pageRemoved(QUuid pageId, int index); // Send ID of removed page
    void pageNameChanged(PageData* page);
    void activePageChanged(PageData* page, int index);
    void pageOrderChanged(); // If we implement reordering

    // Zone signals
    void zoneAddedToPage(PageData* page, ZoneData* zone);
    void zoneRemovedFromPage(PageData* page, QUuid zoneId);
    void zoneDataChanged(ZoneData* zone);
    void pagePropertiesChanged(PageData* page); // For wallpaper/overlay changes

public: // Zone management methods
    ZoneData* addZoneToActivePage(const QString& title, const QRectF& geometry, const QColor& backgroundColor);
    ZoneData* addZoneToPage(const QUuid& pageId, const QString& title, const QRectF& geometry, const QColor& backgroundColor);
    bool removeZoneFromActivePage(const QUuid& zoneId);
    bool removeZoneFromPage(const QUuid& pageId, const QUuid& zoneId);
    void updateZoneData(ZoneData* zone); // To be called when a ZoneWidget's data is changed by user interaction

    void addLoadedPage(PageData* pageData); // For DatabaseManager
    void clearAllPages();                   // For DatabaseManager

    bool renamePage(const QUuid& pageId, const QString& newName);
    void movePage(int fromIndex, int toIndex);
    void notifyPagePropertiesChanged(PageData* page); // To signal UI refresh & save trigger


private:
    QList<PageData*> m_pages;
    int m_activePageIndex;
};

#endif // PAGEMANAGER_H
