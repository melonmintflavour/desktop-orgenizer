#ifndef PAGETABCONTENTWIDGET_H
#define PAGETABCONTENTWIDGET_H

#include <QWidget>
#include <QUuid>
#include <QList> // For storing ZoneWidgets

class ZoneWidget; // Forward declaration
class PageManager; // Forward declaration
class PageData;    // Forward declaration
class ZoneData;    // Forward declaration

class PageTabContentWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PageTabContentWidget(PageData* pageData, PageManager* pageManager, QWidget *parent = nullptr);
    ~PageTabContentWidget() override;

    QUuid pageId() const;
    PageData* pageData() const { return m_pageData; }

public slots:
    void handleZoneAdded(PageData* page, ZoneData* zoneData);
    void handleZoneRemoved(PageData* page, QUuid zoneId);
    void handleZoneDataChanged(ZoneData* zoneData); // To update existing ZoneWidget

private:
    void loadInitialZones();
    ZoneWidget* findZoneWidget(const QUuid& zoneId);

    PageData* m_pageData;       // Reference to the page data this widget displays
    PageManager* m_pageManager; // To interact with (e.g. for ZoneWidget context menus)
    QList<ZoneWidget*> m_zoneWidgets;
};

#endif // PAGETABCONTENTWIDGET_H
