#include "PageTabContentWidget.h"
#include "ZoneWidget.h"
#include "PageData.h"
#include "ZoneData.h"
#include "PageManager.h"
#include <QDebug>
#include <QVBoxLayout>
#include <QPainter> // For paintEvent
#include <QPixmap>  // For wallpaper

PageTabContentWidget::PageTabContentWidget(PageData* pageData, PageManager* pageManager, QWidget *parent)
    : QWidget(parent), m_pageData(pageData), m_pageManager(pageManager)
{
    Q_ASSERT(m_pageData);
    Q_ASSERT(m_pageManager);

    // Set object name for debugging if needed
    setObjectName(QString("PageTabContent_%1").arg(m_pageData->id().toString()));

    // This widget itself does not use a traditional layout for zones.
    // Zones are ZoneWidgets and position themselves using setGeometry.
    // However, we might want a base layout for other potential elements later.
    // For now, it's just a container.

    // Connect to PageManager signals for zone changes on *this specific page*
    // Note: These connections are made from MainWindow when this widget is created for a tab.
    // It's cleaner if MainWindow (or the creator of PageTabContentWidget) manages these
    // connections because PageManager signals are global for all pages.
    // The slots here (handleZoneAdded, etc.) will be called by MainWindow.

    loadInitialZones();
}

PageTabContentWidget::~PageTabContentWidget()
{
    // ZoneWidgets are children of this widget, so Qt should handle their deletion.
    // m_zoneWidgets list just stores pointers, doesn't own them after they are parented.
    qDebug() << "PageTabContentWidget for page ID" << pageId() << "destroyed.";
}

QUuid PageTabContentWidget::pageId() const
{
    return m_pageData ? m_pageData->id() : QUuid();
}

void PageTabContentWidget::loadInitialZones()
{
    if (!m_pageData) return;

    for (ZoneData* zd : m_pageData->zones()) {
        if (zd) {
            ZoneWidget* zw = new ZoneWidget(zd, m_pageManager, this); // Parent is this PageTabContentWidget
            m_zoneWidgets.append(zw);
            zw->show(); // Make sure it's visible
            qDebug() << "Loaded initial zone:" << zd->title() << "on page" << pageId();
        }
    }
}

void PageTabContentWidget::loadPageWallpaper() {
    if (!m_pageData || m_pageData->wallpaperPath().isEmpty()) {
        m_cachedWallpaper = QPixmap();
        m_loadedWallpaperPath.clear();
        update(); // Repaint, as wallpaper might have been cleared
        return;
    }

    if (m_loadedWallpaperPath == m_pageData->wallpaperPath() && !m_cachedWallpaper.isNull()) {
        // Already loaded and path hasn't changed
        return;
    }

    QPixmap pixmap(m_pageData->wallpaperPath());
    if (pixmap.isNull()) {
        qWarning() << "Failed to load page wallpaper:" << m_pageData->wallpaperPath();
        m_cachedWallpaper = QPixmap(); // Ensure it's cleared on failure
    } else {
        m_cachedWallpaper = pixmap;
        qDebug() << "Loaded page wallpaper:" << m_pageData->wallpaperPath();
    }
    m_loadedWallpaperPath = m_pageData->wallpaperPath();
    update(); // Repaint with new/cleared wallpaper
}


void PageTabContentWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    if (!m_pageData) {
        QWidget::paintEvent(event); // Default painting if no pagedata
        return;
    }

    // 1. Check and load wallpaper if path changed
    if (m_loadedWallpaperPath != m_pageData->wallpaperPath() ||
        (!m_pageData->wallpaperPath().isEmpty() && m_cachedWallpaper.isNull())) {
        loadPageWallpaper();
    }

    // 2. Draw wallpaper
    if (!m_cachedWallpaper.isNull()) {
        // Scale to cover, maintaining aspect ratio (Qt::KeepAspectRatioByExpanding)
        // Then it will be clipped by the widget's bounds naturally.
        QPixmap scaledWallpaper = m_cachedWallpaper.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        QPointF pixmapOrigin = QPointF((width() - scaledWallpaper.width()) / 2.0,
                                       (height() - scaledWallpaper.height()) / 2.0);
        painter.drawPixmap(pixmapOrigin, scaledWallpaper);
    } else {
        // If no wallpaper, or failed to load, some fallback background might be desired
        // For now, if main window is transparent, this will be transparent.
        // Or, paint a theme-based default page background if themes are extended to pages.
        // painter.fillRect(rect(), QColor(20,20,20,200)); // Example placeholder
    }

    // 3. Draw overlay color
    if (m_pageData->overlayColor().alpha() > 0) {
        painter.fillRect(rect(), m_pageData->overlayColor());
    }

    // ZoneWidgets are children and will paint themselves on top.
    // No need to call QWidget::paintEvent(event) if we've handled all background.
}

void PageTabContentWidget::handleZoneAdded(PageData* page, ZoneData* zoneData)
{
    if (!m_pageData || !page || page->id() != m_pageData->id() || !zoneData) {
        return; // Not for this page or invalid data
    }

    if (findZoneWidget(zoneData->id())) {
        qWarning() << "ZoneWidget for ID" << zoneData->id() << "already exists on page" << pageId();
        return;
    }

    ZoneWidget* newZoneWidget = new ZoneWidget(zoneData, m_pageManager, this);
    m_zoneWidgets.append(newZoneWidget);
    newZoneWidget->show(); // Important: make the new widget visible
    newZoneWidget->raise(); // Bring to front if overlapping
    qDebug() << "Added ZoneWidget for zone" << zoneData->title() << "ID" << zoneData->id() << "to page" << pageId();
    update(); // Repaint parent to ensure it's all good
}

void PageTabContentWidget::handleZoneRemoved(PageData* page, QUuid zoneId)
{
    if (!m_pageData || !page || page->id() != m_pageData->id()) {
        return; // Not for this page
    }

    for (int i = 0; i < m_zoneWidgets.size(); ++i) {
        if (m_zoneWidgets.at(i)->data()->id() == zoneId) {
            ZoneWidget* zw = m_zoneWidgets.takeAt(i);
            qDebug() << "Removing ZoneWidget for zone ID" << zoneId << "from page" << pageId();
            zw->deleteLater(); // Safe deletion
            update(); // Repaint parent
            return;
        }
    }
    qWarning() << "ZoneWidget for ID" << zoneId << "not found for removal on page" << pageId();
}

void PageTabContentWidget::handleZoneDataChanged(ZoneData* zoneData)
{
    if (!zoneData) return;

    ZoneWidget* zw = findZoneWidget(zoneData->id());
    if (zw) {
        // Check if the zone still belongs to this page, though m_pageManager should emit this
        // only if the zone is relevant.
        if (m_pageData && m_pageData->zoneById(zoneData->id())) {
             qDebug() << "Updating ZoneWidget for zone" << zoneData->title() << "ID" << zoneData->id();
            zw->updateFromData(); // ZoneWidget updates its geometry and repaints
        } else {
            qWarning() << "Received ZoneDataChanged for zone" << zoneData->id() << "but it's not on current page" << pageId();
        }
    } else {
         qWarning() << "Received ZoneDataChanged for zone" << zoneData->id() << "but no matching ZoneWidget found on page" << pageId();
    }
}


ZoneWidget* PageTabContentWidget::findZoneWidget(const QUuid& zoneId)
{
    for (ZoneWidget* zw : m_zoneWidgets) {
        if (zw->data() && zw->data()->id() == zoneId) {
            return zw;
        }
    }
    return nullptr;
}

void PageTabContentWidget::filterIcons(const QString& filterText)
{
    qDebug() << "PageTabContentWidget for page" << pageId() << "filtering icons with text:" << filterText;
    for (ZoneWidget* zoneWidget : m_zoneWidgets) {
        if (zoneWidget) {
            zoneWidget->filterIcons(filterText);
        }
    }
}
