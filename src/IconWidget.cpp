#include "IconWidget.h"
#include "IconData.h"
#include "ZoneWidget.h"   // For accessing parent zone for bounds, etc.
#include "ZoneData.h"     // For m_parentZoneWidget->data()
#include "PageManager.h"  // For notifying changes

#include <QPainter>
#include <QMouseEvent>
#include <QGuiApplication>
#include <QScreen>
#include <QDebug>
#include <QFileInfo> // For display name
#include <QDesktopServices> // For launching later
#include <QUrl>


IconWidget::IconWidget(IconData* iconData, PageManager* pageManager, ZoneWidget *parentZoneWidget)
    : QWidget(parentZoneWidget), // Parent is the ZoneWidget
      m_iconData(iconData),
      m_pageManager(pageManager),
      m_parentZoneWidget(parentZoneWidget),
      m_isDragging(false)
{
    Q_ASSERT(m_iconData);
    Q_ASSERT(m_pageManager);
    Q_ASSERT(m_parentZoneWidget);

    // Set initial geometry and size
    // Icons are typically small, e.g., 64x64 or 32x32 plus text
    // For now, fixed size. Will be dynamic later based on icon image and text.
    setFixedSize(80, 60); // Width, Height (enough for a small icon and text line)
    updateFromData(); // Sets initial position

    setToolTip(m_iconData->filePath());
    setCursor(Qt::PointingHandCursor);

    // Context menu for removing icon
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, [this](const QPoint &pos){
        QMenu contextMenu(this);
        QAction *removeAction = contextMenu.addAction("Remove Icon");
        connect(removeAction, &QAction::triggered, this, &IconWidget::removeIconRequested);
        // QAction *launchAction = contextMenu.addAction("Launch"); // For later
        // connect(launchAction, &QAction::triggered, this, &IconWidget::launchFileRequested);
        contextMenu.exec(mapToGlobal(pos));
    });
}

IconWidget::~IconWidget()
{
    qDebug() << "IconWidget for" << (m_iconData ? m_iconData->filePath() : "Unknown") << "destroyed";
    // IconData is owned by ZoneData, not by IconWidget
}

void IconWidget::updateFromData()
{
    if (!m_iconData) return;
    move(m_iconData->positionInZone().toPoint());
    update(); // Trigger repaint for name change or visual state
}

void IconWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (!m_iconData) return;

    // Simple background (optional, can be transparent)
    // painter.fillRect(rect(), QColor(200, 200, 200, 50));

    // Placeholder for icon image (e.g., a generic file icon)
    // For now, just a rectangle
    QRectF iconRect(width()/2.0 - 16, 5, 32, 32); // Centered 32x32 icon placeholder
    painter.setBrush(Qt::lightGray);
    painter.setPen(Qt::darkGray);
    painter.drawRoundedRect(iconRect, 4, 4);
    // painter.drawPixmap(iconRect.topLeft(), m_iconData->cachedIcon()); // For later

    // Display name
    painter.setPen(Qt::white); // Adjust text color as needed
    QFont font = painter.font();
    font.setPointSize(8);
    painter.setFont(font);

    QRectF textRect(0, iconRect.bottom() + 2, width(), height() - (iconRect.bottom() + 2) - 2);
    painter.drawText(textRect, Qt::AlignCenter | Qt::TextWordWrap, m_iconData->displayName());
}

void IconWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_isDragging = true;
        m_dragStartPosition = event->position().toPoint(); // Position of click relative to widget's top-left
        raise(); // Bring to front while dragging
        event->accept();
    } else {
        QWidget::mousePressEvent(event);
    }
}

void IconWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isDragging && (event->buttons() & Qt::LeftButton)) {
        // Calculate new top-left position for the widget, relative to its parent (ZoneWidget)
        QPoint newPos = mapToParent(event->position().toPoint()) - m_dragStartPosition;

        // Constrain within parent (ZoneWidget) bounds
        if (m_parentZoneWidget) {
            int constrainedX = qBound(0, newPos.x(), m_parentZoneWidget->width() - width());
            int constrainedY = qBound(0, newPos.y(), m_parentZoneWidget->height() - height());
            newPos = QPoint(constrainedX, constrainedY);
        }

        move(newPos);
        event->accept();
    } else {
        QWidget::mouseMoveEvent(event);
    }
}

void IconWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_isDragging && event->button() == Qt::LeftButton) {
        m_isDragging = false;
        if (m_iconData) {
            QPointF newPositionInZone = pos(); // Current position is relative to parent (ZoneWidget)
            if (m_iconData->positionInZone() != newPositionInZone) {
                m_iconData->setPositionInZone(newPositionInZone);
                qDebug() << "Icon" << m_iconData->id() << "moved to" << newPositionInZone << "in zone" << m_parentZoneWidget->data()->id();
                // Notify PageManager (via ZoneData) that data has changed
                if (m_parentZoneWidget && m_parentZoneWidget->data()) {
                    m_pageManager->updateZoneData(m_parentZoneWidget->data());
                }
            }
        }
        event->accept();
    } else {
        QWidget::mouseReleaseEvent(event);
    }
}

void IconWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_iconData) {
        qDebug() << "Icon double-clicked (launch later):" << m_iconData->filePath();
        // QDesktopServices::openUrl(QUrl::fromLocalFile(m_iconData->filePath())); // Implement launching later
        event->accept();
    } else {
        QWidget::mouseDoubleClickEvent(event);
    }
}

void IconWidget::contextMenuEvent(QContextMenuEvent *event)
{
    // This is overridden to ensure we handle it, but actual menu creation is
    // via setContextMenuPolicy(Qt::CustomContextMenu) and the connected slot.
    QWidget::contextMenuEvent(event);
}

void IconWidget::removeIconRequested()
{
    if (!m_iconData || !m_parentZoneWidget || !m_parentZoneWidget->data() || !m_pageManager) return;

    ZoneData* zd = m_parentZoneWidget->data();
    QUuid iconIdToRemove = m_iconData->id();

    qDebug() << "Requesting removal of icon" << m_iconData->displayName() << "ID" << iconIdToRemove
             << "from zone" << zd->title() << "ID" << zd->id();

    if (zd->removeIcon(iconIdToRemove)) { // This also deletes IconData from ZoneData's list
        m_pageManager->updateZoneData(zd); // Notify that zone content changed
        // The IconWidget itself will be deleted by ZoneWidget when it handles this change
        // or it can be done here: this->deleteLater();
        // For now, let ZoneWidget handle its child widget removal when it processes updateZoneData.
    } else {
        qWarning() << "Failed to remove icon from ZoneData. Icon ID:" << iconIdToRemove;
    }
}
