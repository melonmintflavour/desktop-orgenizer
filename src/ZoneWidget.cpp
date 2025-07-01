#include "ZoneWidget.h"
#include "ZoneData.h"
#include "PageManager.h" // For emitting zoneDataChanged
#include <QPainter>
#include <QMouseEvent>
#include <QGuiApplication> // For screen geometry or cursor control
#include <QScreen>
#include <QDebug>
#include <QApplication> // For qApp->setOverrideCursor
#include <QMimeData>    // For drag and drop
#include <QUrl>         // For file paths from drop
#include "IconWidget.h" // For creating IconWidgets
#include "IconData.h"   // For creating IconData
#include "PageTabContentWidget.h" // For qobject_cast to get parent PageData

ZoneWidget::ZoneWidget(ZoneData* zoneData, PageManager* pageManager, QWidget *parent)
    : QWidget(parent), m_zoneData(zoneData), m_pageManager(pageManager),
      m_isResizing(false), m_isMoving(false), m_currentResizeRegion(ResizeRegion::None)
{
    Q_ASSERT(m_zoneData);
    Q_ASSERT(m_pageManager);

    // Set initial geometry from ZoneData
    setGeometry(m_zoneData->geometry().toRect());
    setMinimumSize(50, 30); // Minimum sensible size for a zone

    // Enable mouse tracking to get mouseMoveEvents even when no button is pressed (for cursor changes)
    setMouseTracking(true);
    setAcceptDrops(true); // Enable drag and drop onto this widget

    // Custom context menu for removing the zone
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, [this](const QPoint &pos){
        QMenu contextMenu(this);
        QAction *removeAction = contextMenu.addAction("Remove Zone");
        connect(removeAction, &QAction::triggered, this, &ZoneWidget::removeZoneRequested);
        contextMenu.exec(mapToGlobal(pos));
    });

    loadOrUpdateIcons(); // Load initial icons
}

ZoneWidget::~ZoneWidget()
{
    // IconWidgets are children of this, Qt handles their deletion.
    // ZoneData is owned by PageManager/PageData.
    qDebug() << "ZoneWidget for" << (m_zoneData ? m_zoneData->title() : "Unknown") << "destroyed";
}

void ZoneWidget::updateFromData()
{
    if (!m_zoneData) return;
    setGeometry(m_zoneData->geometry().toRect());
    loadOrUpdateIcons(); // This will also trigger repaint of children if needed
    update(); // Trigger repaint for zone itself
}


void ZoneWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (!m_zoneData) return;

    // Background
    painter.fillRect(rect(), m_zoneData->backgroundColor());

    // Border (optional, for visual clarity during development)
    painter.setPen(QPen(Qt::gray, 1));
    painter.drawRect(rect().adjusted(0, 0, -1, -1));

    // Title
    painter.setPen(Qt::white); // Adjust text color as needed
    QFont font = painter.font();
    font.setPointSize(10);
    // font.setBold(true); // Optional
    painter.setFont(font);

    QRectF textRect = rect().adjusted(5, 2, -5, -2); // Margins for text
    // Simple title bar area (top part of the zone)
    QRectF titleBarRect(0, 0, width(), qMin(20, height())); // Max 20px high title bar
    painter.drawText(titleBarRect.adjusted(5,0,-5,0), Qt::AlignLeft | Qt::AlignVCenter, m_zoneData->title());

    // If resizing, could draw resize handles
    // if (m_isResizing || underMouse()) { // Or always if underMouse()
    //     painter.setBrush(Qt::black);
    //     // Example: Bottom-right handle
    //     // painter.drawRect(width() - RESIZE_BORDER_SENSITIVITY, height() - RESIZE_BORDER_SENSITIVITY, RESIZE_BORDER_SENSITIVITY, RESIZE_BORDER_SENSITIVITY);
    // }
}

void ZoneWidget::mousePressEvent(QMouseEvent *event)
{
    if (!m_zoneData) return;

    if (event->button() == Qt::LeftButton) {
        m_mousePressPosition = event->globalPosition().toPoint();
        m_dragStartPosition = event->position().toPoint();
        m_originalGeometry = geometry(); // Use current widget geometry which should match ZoneData

        m_currentResizeRegion = getResizeRegion(event->position().toPoint());

        if (m_currentResizeRegion != ResizeRegion::None && m_currentResizeRegion != ResizeRegion::Move) {
            m_isResizing = true;
            qDebug() << "Starting resize op:" << m_currentResizeRegion;
        } else {
            // Check if click is on title bar area for moving
            QRectF titleBarRect(0, 0, width(), qMin(20, height()));
            if (titleBarRect.contains(event->position())) {
                 m_isMoving = true;
                 m_currentResizeRegion = ResizeRegion::Move; // Ensure region is set for move
                 qDebug() << "Starting move op";
            } else {
                // If not on title bar and not a resize region, treat as non-moving click for now
                // Could be used for selection later
                m_currentResizeRegion = ResizeRegion::None;
            }
        }
        event->accept();
    } else {
        QWidget::mousePressEvent(event);
    }
}

void ZoneWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_zoneData) return;

    if (m_isResizing) {
        handleResize(event->globalPosition().toPoint());
        event->accept();
    } else if (m_isMoving) {
        handleMove(event->globalPosition().toPoint());
        event->accept();
    } else {
        // Update cursor even if not dragging/resizing
        updateCursorShape(event->position().toPoint());
        QWidget::mouseMoveEvent(event);
    }
}

void ZoneWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_zoneData) return;

    if (event->button() == Qt::LeftButton) {
        if (m_isResizing || m_isMoving) {
            m_isResizing = false;
            m_isMoving = false;

            // Update ZoneData with the new geometry
            m_zoneData->setGeometry(QRectF(geometry()));
            // Notify PageManager that data has changed (so it can be saved, etc.)
            m_pageManager->updateZoneData(m_zoneData);

            qDebug() << "Finished move/resize. New geometry:" << geometry();
            unsetCursor(); // Reset cursor to normal arrow
            m_currentResizeRegion = ResizeRegion::None;
        }
        event->accept();
    } else {
        QWidget::mouseReleaseEvent(event);
    }
}

void ZoneWidget::enterEvent(QEnterEvent *event)
{
    Q_UNUSED(event);
    // update(); // Could repaint to show handles if desired
}

void ZoneWidget::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    if (!m_isResizing && !m_isMoving) { // Don't unset cursor if an operation is in progress
        unsetCursor();
    }
    // update();
}

ZoneWidget::ResizeRegion ZoneWidget::getResizeRegion(const QPoint& pos)
{
    int x = pos.x();
    int y = pos.y();
    int w = width();
    int h = height();

    bool onLeft = x >= 0 && x < RESIZE_BORDER_SENSITIVITY;
    bool onRight = x >= w - RESIZE_BORDER_SENSITIVITY && x < w;
    bool onTop = y >= 0 && y < RESIZE_BORDER_SENSITIVITY;
    bool onBottom = y >= h - RESIZE_BORDER_SENSITIVITY && y < h;

    if (onTop && onLeft) return ResizeRegion::TopLeft;
    if (onTop && onRight) return ResizeRegion::TopRight;
    if (onBottom && onLeft) return ResizeRegion::BottomLeft;
    if (onBottom && onRight) return ResizeRegion::BottomRight;
    if (onTop) return ResizeRegion::Top;
    if (onBottom) return ResizeRegion::Bottom;
    if (onLeft) return ResizeRegion::Left;
    if (onRight) return ResizeRegion::Right;

    // Check for move region (title bar)
    // QRectF titleBarRect(0, 0, width(), qMin(20, height()));
    // if (titleBarRect.contains(pos)) return ResizeRegion::Move;
    // Defaulted to move if not a resize region in mousePressEvent based on title bar.

    return ResizeRegion::None; // No specific resize/move region based on simple border check
}

void ZoneWidget::updateCursorShape(const QPoint& pos)
{
    if (m_isResizing || m_isMoving) return; // Cursor is likely set by the operation

    ResizeRegion region = getResizeRegion(pos);
    Qt::CursorShape cursor = Qt::ArrowCursor;

    switch (region) {
        case ResizeRegion::Top:
        case ResizeRegion::Bottom:
            cursor = Qt::SizeVerCursor;
            break;
        case ResizeRegion::Left:
        case ResizeRegion::Right:
            cursor = Qt::SizeHorCursor;
            break;
        case ResizeRegion::TopLeft:
        case ResizeRegion::BottomRight:
            cursor = Qt::SizeFDiagCursor;
            break;
        case ResizeRegion::TopRight:
        case ResizeRegion::BottomLeft:
            cursor = Qt::SizeBDiagCursor;
            break;
        case ResizeRegion::Move: // Handled by title bar check in mousePress
             // QRectF titleBarRect(0, 0, width(), qMin(20, height()));
             // if (titleBarRect.contains(pos)) cursor = Qt::SizeAllCursor; // Or OpenHandCursor
            break;
        default:
            // Check for move cursor if over title bar
            QRectF titleBarRect(0, 0, width(), qMin(20, height()));
            if (titleBarRect.contains(pos)) {
                 cursor = Qt::SizeAllCursor; // Or OpenHandCursor
            } else {
                cursor = Qt::ArrowCursor;
            }
            break;
    }
    setCursor(cursor);
}


void ZoneWidget::handleMove(const QPoint& newMouseGlobalPos)
{
    if (!m_isMoving) return;
    QPoint delta = newMouseGlobalPos - m_mousePressPosition;
    // New top-left position for the widget, relative to its parent.
    // m_originalGeometry is the geometry of the widget itself at the start of the move.
    // m_dragStartPosition is the click position *within* the widget.
    // So, the new widget top-left is original_widget_topLeft + (current_mouse_global - original_mouse_global_press)

    QPoint newTopLeft = m_originalGeometry.topLeft().toPoint() + delta;

    // Ensure it stays within parent bounds (optional, but good practice)
    // if (parentWidget()) {
    //     newTopLeft.setX(qBound(0, newTopLeft.x(), parentWidget()->width() - width()));
    //     newTopLeft.setY(qBound(0, newTopLeft.y(), parentWidget()->height() - height()));
    // }

    move(newTopLeft);
    // Geometry in ZoneData will be updated on mouseRelease
}

void ZoneWidget::handleResize(const QPoint& newMouseGlobalPos)
{
    if (!m_isResizing) return;

    QRect newGeometry = m_originalGeometry.toRect();
    QPoint delta = newMouseGlobalPos - m_mousePressPosition; // How much mouse moved globally

    // Adjust geometry based on which handle is being dragged
    // Note: Calculations are based on global mouse movement applied to original geometry
    switch (m_currentResizeRegion) {
        case ResizeRegion::TopLeft:
            newGeometry.setTopLeft(m_originalGeometry.topLeft().toPoint() + delta);
            break;
        case ResizeRegion::Top:
            newGeometry.setTop(m_originalGeometry.top() + delta.y());
            break;
        case ResizeRegion::TopRight:
            newGeometry.setTop(m_originalGeometry.top() + delta.y());
            newGeometry.setRight(m_originalGeometry.right() + delta.x());
            break;
        case ResizeRegion::Left:
            newGeometry.setLeft(m_originalGeometry.left() + delta.x());
            break;
        case ResizeRegion::Right:
            newGeometry.setRight(m_originalGeometry.right() + delta.x());
            break;
        case ResizeRegion::BottomLeft:
            newGeometry.setBottom(m_originalGeometry.bottom() + delta.y());
            newGeometry.setLeft(m_originalGeometry.left() + delta.x());
            break;
        case ResizeRegion::Bottom:
            newGeometry.setBottom(m_originalGeometry.bottom() + delta.y());
            break;
        case ResizeRegion::BottomRight:
            newGeometry.setBottomRight(m_originalGeometry.bottomRight().toPoint() + delta);
            break;
        default:
            return; // Should not happen if m_isResizing is true
    }

    // Enforce minimum size
    if (newGeometry.width() < minimumSize().width()) {
        if (m_currentResizeRegion == ResizeRegion::TopLeft || m_currentResizeRegion == ResizeRegion::Left || m_currentResizeRegion == ResizeRegion::BottomLeft) {
            newGeometry.setLeft(newGeometry.right() - minimumSize().width());
        } else {
            newGeometry.setWidth(minimumSize().width());
        }
    }
    if (newGeometry.height() < minimumSize().height()) {
         if (m_currentResizeRegion == ResizeRegion::TopLeft || m_currentResizeRegion == ResizeRegion::Top || m_currentResizeRegion == ResizeRegion::TopRight) {
            newGeometry.setTop(newGeometry.bottom() - minimumSize().height());
        } else {
            newGeometry.setHeight(minimumSize().height());
        }
    }

    // Update widget's geometry
    // The geometry is relative to the parent, which is PageTabContentWidget
    setGeometry(newGeometry);
    // Actual ZoneData geometry is updated on mouseRelease
}

void ZoneWidget::contextMenuEvent(QContextMenuEvent *event)
{
    // This is overridden to ensure we handle it, but actual menu creation is
    // via setContextMenuPolicy(Qt::CustomContextMenu) and the connected slot.
    // We could also create the menu directly here.
    QWidget::contextMenuEvent(event);
}

void ZoneWidget::removeZoneRequested()
{
    if (!m_zoneData || !m_pageManager) return;

    // Find the parent PageData for this zone to correctly signal PageManager
    // This requires iterating through pages in PageManager, or PageManager providing a way
    // to find the parent page of a zone.
    // For now, let's assume the ZoneWidget is on the active page for removal.
    // This might need refinement if zones could be managed on non-active pages directly.
    PageData* parentPageData = nullptr;
    if (parentWidget() && qobject_cast<PageTabContentWidget*>(parentWidget())) {
        parentPageData = qobject_cast<PageTabContentWidget*>(parentWidget())->pageData();
    }

    if (parentPageData) {
        qDebug() << "Requesting removal of zone" << m_zoneData->title() << "ID" << m_zoneData->id() << "from page" << parentPageData->id();
        m_pageManager->removeZoneFromPage(parentPageData->id(), m_zoneData->id());
        // The ZoneWidget itself will be deleted by its parent (PageTabContentWidget)
        // when it receives the zoneRemovedFromPage signal.
    } else {
        qWarning() << "Cannot determine parent page for zone removal. Zone ID:" << m_zoneData->id();
        // Fallback: try removing from active page if no direct parent found (less robust)
        PageData* activePage = m_pageManager->activePage();
        if (activePage && activePage->zoneById(m_zoneData->id())) {
             qDebug() << "Fallback: Requesting removal of zone" << m_zoneData->title() << "from active page" << activePage->id();
             m_pageManager->removeZoneFromPage(activePage->id(), m_zoneData->id());
        } else {
            qWarning() << "Cannot remove zone, parent page context missing or zone not on active page.";
        }
    }
}

// --- Icon Management ---

void ZoneWidget::loadOrUpdateIcons() {
    if (!m_zoneData) return;

    // Sync m_iconWidgets with m_zoneData->icons()
    QList<QUuid> currentIconDataIds;
    for (IconData* id : m_zoneData->icons()) {
        currentIconDataIds.append(id->id());
    }

    // Remove IconWidgets for icons that no longer exist in data
    for (int i = m_iconWidgets.size() - 1; i >= 0; --i) {
        IconWidget* iw = m_iconWidgets.at(i);
        if (!iw->data() || !currentIconDataIds.contains(iw->data()->id())) {
            m_iconWidgets.removeAt(i);
            iw->deleteLater();
            qDebug() << "Removed IconWidget for stale/missing IconData ID:" << (iw->data() ? iw->data()->id().toString() : "Unknown");
        }
    }

    // Add/Update IconWidgets
    for (IconData* iconD : m_zoneData->icons()) {
        IconWidget* existingWidget = findIconWidget(iconD->id());
        if (existingWidget) {
            existingWidget->updateFromData(); // Update position or other visuals
        } else {
            IconWidget* newIconWidget = new IconWidget(iconD, m_pageManager, this);
            m_iconWidgets.append(newIconWidget);
            newIconWidget->show();
            qDebug() << "Created IconWidget for IconData ID:" << iconD->id() << "Path:" << iconD->filePath();
        }
    }
    update(); // Repaint zone if icon changes might affect it (e.g. bounds checks)
}

IconWidget* ZoneWidget::findIconWidget(const QUuid& iconId) {
    for (IconWidget* iw : m_iconWidgets) {
        if (iw->data() && iw->data()->id() == iconId) {
            return iw;
        }
    }
    return nullptr;
}


// --- Drag and Drop ---
void ZoneWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
        qDebug() << "ZoneWidget: Drag Enter accepted for URLs";
    } else {
        event->ignore();
        qDebug() << "ZoneWidget: Drag Enter ignored - no URLs";
    }
}

void ZoneWidget::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void ZoneWidget::dropEvent(QDropEvent *event)
{
    if (!m_zoneData) {
        event->ignore();
        return;
    }

    const QMimeData* mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QList<QUrl> urls = mimeData->urls();
        for (const QUrl& url : urls) {
            if (url.isLocalFile()) {
                QString filePath = url.toLocalFile();
                QPointF dropPos = event->position(); // Position relative to ZoneWidget

                // Create IconData
                IconData* newIconData = new IconData(filePath, dropPos);
                m_zoneData->addIcon(newIconData); // ZoneData now owns IconData

                // Create IconWidget (or let loadOrUpdateIcons handle it)
                // IconWidget* newIconWidget = new IconWidget(newIconData, m_pageManager, this);
                // m_iconWidgets.append(newIconWidget);
                // newIconWidget->show();
                // newIconWidget->raise();
                // No, better to just update the data and let loadOrUpdateIcons sync

                qDebug() << "Dropped file:" << filePath << "at" << dropPos << "in zone" << m_zoneData->id();
            }
        }
        // Notify PageManager that ZoneData has changed (icons added)
        m_pageManager->updateZoneData(m_zoneData);
        // loadOrUpdateIcons(); // updateZoneData signal should eventually lead to this via PageTabContentWidget -> ZoneWidget::updateFromData
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}
