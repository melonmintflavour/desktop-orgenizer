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
#include <QInputDialog> // For title editing
#include <QColorDialog> // For color picking
#include <QFileDialog>  // For selecting background image
#include <QPainterPath> // For rounded rect clipping
#include <QGraphicsBlurEffect> // For blurring image
#include <QGraphicsScene>      // For using QGraphicsBlurEffect
#include <QGraphicsPixmapItem> // For using QGraphicsBlurEffect
#include <QGraphicsDropShadowEffect> // For drop shadows
#include "IconWidget.h" // For creating IconWidgets
#include "IconData.h"   // For creating IconData
#include "PageTabContentWidget.h" // For qobject_cast to get parent PageData
#include "ThemeManager.h" // For text color based on theme

ZoneWidget::ZoneWidget(ZoneData* zoneData, PageManager* pageManager, QWidget *parent)
    : QWidget(parent), m_zoneData(zoneData), m_pageManager(pageManager),
      m_isResizing(false), m_isMoving(false), m_currentResizeRegion(ResizeRegion::None),
      m_lastBlurState(false) // Initialize last blur state
{
    Q_ASSERT(m_zoneData);
    Q_ASSERT(m_pageManager);

    setObjectName("ZoneWidget"); // For stylesheet targeting
    // Set initial geometry from ZoneData
    setGeometry(m_zoneData->geometry().toRect());
    setMinimumSize(50, 30); // Minimum sensible size for a zone

    // Enable mouse tracking to get mouseMoveEvents even when no button is pressed (for cursor changes)
    setMouseTracking(true);
    setAcceptDrops(true); // Enable drag and drop onto this widget

    // Add drop shadow effect
    QGraphicsDropShadowEffect* shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setBlurRadius(15);
    shadowEffect->setColor(QColor(0, 0, 0, 100)); // Semi-transparent black
    shadowEffect->setOffset(4, 4);               // Offset to bottom-right
    setGraphicsEffect(shadowEffect);

    // Custom context menu
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, [this](const QPoint &pos){
        QMenu contextMenu(this);
        QAction *renameAction = contextMenu.addAction("Rename Zone...");
        connect(renameAction, &QAction::triggered, this, &ZoneWidget::renameZoneRequested);

        QAction *bgColorAction = contextMenu.addAction("Change Background Color...");
        connect(bgColorAction, &QAction::triggered, this, &ZoneWidget::changeBackgroundColorRequested);

        QAction *cornerRadiusAction = contextMenu.addAction("Set Corner Radius...");
        connect(cornerRadiusAction, &QAction::triggered, this, &ZoneWidget::setCornerRadiusRequested);

        contextMenu.addSeparator();
        QMenu* bgImageMenu = contextMenu.addMenu("Background Image");
        QAction *setBgImageAction = bgImageMenu->addAction("Set Image...");
        connect(setBgImageAction, &QAction::triggered, this, &ZoneWidget::setBackgroundImageRequested);

        if (!m_zoneData->backgroundImagePath().isEmpty()) {
            QAction *clearBgImageAction = bgImageMenu->addAction("Clear Image");
            connect(clearBgImageAction, &QAction::triggered, this, &ZoneWidget::clearBackgroundImageRequested);

            QAction *blurBgImageAction = bgImageMenu->addAction("Toggle Image Blur");
            blurBgImageAction->setCheckable(true);
            blurBgImageAction->setChecked(m_zoneData->blurBackgroundImage());
            connect(blurBgImageAction, &QAction::triggered, this, &ZoneWidget::toggleBlurBackgroundImageRequested);
        }

        contextMenu.addSeparator();
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

    // Prepare background image if path changed or not loaded
    if (m_loadedBgImagePath != m_zoneData->backgroundImagePath() || m_cachedBgPixmap.isNull()) {
        loadBackgroundImage(); // This will also prepare m_processedBgPixmap
    } else if (!m_cachedBgPixmap.isNull() &&
               (m_processedBgPixmap.isNull() || m_cachedBgPixmap.size() != m_processedBgPixmap.size() || m_zoneData->blurBackgroundImage() != m_lastBlurState)) {
        // Re-process if blur state changed or pixmap sizes mismatch (e.g. after resize)
        prepareProcessedBackgroundImage();
    }


    QPainterPath clipPath;
    clipPath.addRoundedRect(rect(), m_zoneData->cornerRadius(), m_zoneData->cornerRadius());

    painter.setClipPath(clipPath); // Clip drawing to the rounded rect

    // Draw background color first
    painter.fillPath(clipPath, m_zoneData->backgroundColor());

    // Draw background image if available and processed
    if (!m_processedBgPixmap.isNull()) {
        // Scale pixmap to fill the zone rect, maintaining aspect ratio and cropping if necessary.
        // Qt::KeepAspectRatioByExpanding will make sure it covers, then clipping handles rounded corners.
        QPixmap scaledPixmap = m_processedBgPixmap.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        QPointF pixmapOrigin = QPointF((width() - scaledPixmap.width()) / 2.0, (height() - scaledPixmap.height()) / 2.0);
        painter.drawPixmap(pixmapOrigin, scaledPixmap);
    }

    // Border
    painter.setPen(QPen(Qt::gray, 1)); // Border color from theme or data later
    painter.drawPath(clipPath);


    // Title - Ensure text is drawn *after* clipping and background fill
    painter.setClipping(false); // Disable clipping for text

    // Determine text color based on effective background (image or color)
    // This is a simple heuristic. A more robust way might involve analyzing average color under text.
    QColor effectiveBgColor = m_zoneData->backgroundColor();
    if (!m_processedBgPixmap.isNull()) {
        // If there's an image, assume it might be varied. A fixed contrasting color might be better.
        // For now, let's try a fixed color or one that contrasts with a semi-transparent overlay.
        // Or, allow user to set title text color.
        // As a simple approach, if image is present, use white/black based on a general assumption or theme.
         effectiveBgColor = QColor(128,128,128); // Neutral grey to pick text color against
    }
    QColor textColor = (effectiveBgColor.lightnessF() < 0.5) ? Qt::white : Qt::black;
    // If a theme is active, it might override this. For now, direct contrast.
    // If Zone bg is very transparent, text color should contrast with what's *behind* the window.
    // This is complex. Let's use theme-provided text color as a base, or a fixed one.
    // The current theme sets ZoneWidget { color: ... }, let's try to use that if this is too complex.
    // For now, stick to simple contrast with the zone's main BG color or a default if image.
    if (!m_processedBgPixmap.isNull()) {
        textColor = (ThemeManager::currentTheme() == ThemeManager::Theme::Dark) ? Qt::white : Qt::black;
    } else {
         textColor = (m_zoneData->backgroundColor().lightnessF() < 0.5) ? Qt::white : Qt::black;
    }
    painter.setPen(textColor);

    QFont font = painter.font();
    font.setPointSize(10);
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

void ZoneWidget::filterIcons(const QString& filterText)
{
    qDebug() << "ZoneWidget" << (m_zoneData ? m_zoneData->title() : "N/A") << "filtering icons with text:" << filterText;
    bool searchIsEmpty = filterText.isEmpty();
    for (IconWidget* iconWidget : m_iconWidgets) {
        if (iconWidget && iconWidget->data()) {
            if (searchIsEmpty) {
                iconWidget->setVisible(true);
            } else {
                bool nameMatch = iconWidget->data()->displayName().contains(filterText, Qt::CaseInsensitive);
                bool pathMatch = iconWidget->data()->filePath().contains(filterText, Qt::CaseInsensitive);
                iconWidget->setVisible(nameMatch || pathMatch);
            }
        }
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

void ZoneWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (!m_zoneData) return;

    QRectF titleBarRect(0, 0, width(), qMin(20, height()));
    if (titleBarRect.contains(event->position())) {
        if (event->button() == Qt::LeftButton) {
            renameZoneRequested(); // Call the same logic as context menu
            event->accept();
            return;
        }
    }
    QWidget::mouseDoubleClickEvent(event);
}


void ZoneWidget::removeZoneRequested()
{
    if (!m_zoneData || !m_pageManager) return;

    PageData* parentPageData = nullptr;
    if (parentWidget() && qobject_cast<PageTabContentWidget*>(parentWidget())) {
        parentPageData = qobject_cast<PageTabContentWidget*>(parentWidget())->pageData();
    }

    if (parentPageData) {
        qDebug() << "Requesting removal of zone" << m_zoneData->title() << "ID" << m_zoneData->id() << "from page" << parentPageData->id();
        m_pageManager->removeZoneFromPage(parentPageData->id(), m_zoneData->id());
    } else {
        qWarning() << "Cannot determine parent page for zone removal. Zone ID:" << m_zoneData->id();
        PageData* activePage = m_pageManager->activePage();
        if (activePage && activePage->zoneById(m_zoneData->id())) {
             qDebug() << "Fallback: Requesting removal of zone" << m_zoneData->title() << "from active page" << activePage->id();
             m_pageManager->removeZoneFromPage(activePage->id(), m_zoneData->id());
        } else {
            qWarning() << "Cannot remove zone, parent page context missing or zone not on active page.";
        }
    }
}

void ZoneWidget::renameZoneRequested() {
    if (!m_zoneData || !m_pageManager) return;

    bool ok;
    QString currentTitle = m_zoneData->title();
    QString newTitle = QInputDialog::getText(this, "Rename Zone", "Enter new zone title:", QLineEdit::Normal, currentTitle, &ok);

    if (ok && !newTitle.isEmpty() && newTitle != currentTitle) {
        m_zoneData->setTitle(newTitle);
        m_pageManager->updateZoneData(m_zoneData); // Notifies for save and repaint
        update(); // Immediate repaint of this widget
        qDebug() << "Zone ID" << m_zoneData->id() << "renamed to" << newTitle;
    }
}

void ZoneWidget::changeBackgroundColorRequested() {
    if (!m_zoneData || !m_pageManager) return;

    QColor newColor = QColorDialog::getColor(m_zoneData->backgroundColor(), this,
                                             "Select Zone Background Color",
                                             QColorDialog::ShowAlphaChannel); // Allow alpha selection

    if (newColor.isValid()) {
        m_zoneData->setBackgroundColor(newColor);
        m_pageManager->updateZoneData(m_zoneData); // Notifies for save and repaint
        update(); // Immediate repaint of this widget
        qDebug() << "Zone ID" << m_zoneData->id() << "background color changed to" << newColor.name(QColor::HexArgb);
    }
}

void ZoneWidget::setCornerRadiusRequested() {
    if (!m_zoneData || !m_pageManager) return;

    bool ok;
    int currentRadius = m_zoneData->cornerRadius();
    int newRadius = QInputDialog::getInt(this, "Set Corner Radius", "Enter corner radius (pixels):", currentRadius, 0, 100, 1, &ok); // Min 0, Max 100, step 1

    if (ok && newRadius != currentRadius) {
        m_zoneData->setCornerRadius(newRadius);
        m_pageManager->updateZoneData(m_zoneData);
        update(); // Repaint for new radius
        qDebug() << "Zone ID" << m_zoneData->id() << "corner radius set to" << newRadius;
    }
}

void ZoneWidget::setBackgroundImageRequested() {
    if (!m_zoneData || !m_pageManager) return;

    QString filePath = QFileDialog::getOpenFileName(this, tr("Select Background Image"),
                                                    QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
                                                    tr("Images (*.png *.jpg *.jpeg *.bmp)"));
    if (!filePath.isEmpty()) {
        m_zoneData->setBackgroundImagePath(filePath);
        m_loadedBgImagePath.clear(); // Force reload
        m_pageManager->updateZoneData(m_zoneData);
        update();
        qDebug() << "Zone ID" << m_zoneData->id() << "background image set to" << filePath;
    }
}

void ZoneWidget::clearBackgroundImageRequested() {
    if (!m_zoneData || !m_pageManager) return;

    if (!m_zoneData->backgroundImagePath().isEmpty()) {
        m_zoneData->setBackgroundImagePath(QString()); // Clear path
        m_zoneData->setBlurBackgroundImage(false); // Also reset blur if image is cleared
        m_cachedBgPixmap = QPixmap(); // Clear cache
        m_processedBgPixmap = QPixmap();
        m_loadedBgImagePath.clear();
        m_pageManager->updateZoneData(m_zoneData);
        update();
        qDebug() << "Zone ID" << m_zoneData->id() << "background image cleared.";
    }
}

void ZoneWidget::toggleBlurBackgroundImageRequested() {
    if (!m_zoneData || !m_pageManager || m_zoneData->backgroundImagePath().isEmpty()) return;

    m_zoneData->setBlurBackgroundImage(!m_zoneData->blurBackgroundImage());
    // No need to clear m_cachedBgPixmap, just re-process it
    m_processedBgPixmap = QPixmap(); // Force re-processing
    m_pageManager->updateZoneData(m_zoneData);
    update();
    qDebug() << "Zone ID" << m_zoneData->id() << "blur background image toggled to" << m_zoneData->blurBackgroundImage();
}


// --- Image Loading and Processing ---
void ZoneWidget::loadBackgroundImage() {
    if (!m_zoneData || m_zoneData->backgroundImagePath().isEmpty()) {
        m_cachedBgPixmap = QPixmap();
        m_processedBgPixmap = QPixmap();
        m_loadedBgImagePath.clear();
        return;
    }

    if (m_loadedBgImagePath == m_zoneData->backgroundImagePath() && !m_cachedBgPixmap.isNull()) {
        // Already loaded and path hasn't changed
        prepareProcessedBackgroundImage(); // Ensure processed one is up-to-date (e.g. blur state)
        return;
    }

    QPixmap pixmap(m_zoneData->backgroundImagePath());
    if (pixmap.isNull()) {
        qWarning() << "Failed to load background image:" << m_zoneData->backgroundImagePath();
        m_cachedBgPixmap = QPixmap();
        m_processedBgPixmap = QPixmap();
        m_loadedBgImagePath.clear();
    } else {
        m_cachedBgPixmap = pixmap;
        m_loadedBgImagePath = m_zoneData->backgroundImagePath();
        qDebug() << "Loaded background image" << m_loadedBgImagePath << "for zone" << m_zoneData->id() << "Size:" << m_cachedBgPixmap.size();
    }
    prepareProcessedBackgroundImage(); // Prepare initial processed version
}

void ZoneWidget::prepareProcessedBackgroundImage() {
    if (m_cachedBgPixmap.isNull()) {
        m_processedBgPixmap = QPixmap();
        m_lastBlurState = m_zoneData ? m_zoneData->blurBackgroundImage() : false;
        return;
    }

    if (m_zoneData->blurBackgroundImage()) {
        // Simple blur: Scale down, then scale up with smooth transform
        // This is a very basic and often not great looking blur.
        // For a better blur, QGraphicsBlurEffect is preferred but requires a scene or more setup.
        // Let's try a QGraphicsBlurEffect approach by rendering to an image.

        QImage sourceImage = m_cachedBgPixmap.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
        QGraphicsScene scene;
        QGraphicsPixmapItem item(QPixmap::fromImage(sourceImage));
        QGraphicsBlurEffect *blur = new QGraphicsBlurEffect;
        blur->setBlurRadius(8); // Adjust blur radius as needed
        item.setGraphicsEffect(blur);
        scene.addItem(&item);

        QImage resultImage(sourceImage.size(), QImage::Format_ARGB32_Premultiplied);
        resultImage.fill(Qt::transparent);
        QPainter p(&resultImage);
        scene.render(&p, QRectF(), QRectF(0,0,sourceImage.width(), sourceImage.height()));
        p.end();

        m_processedBgPixmap = QPixmap::fromImage(resultImage);
        qDebug() << "Applied blur to background image for zone" << m_zoneData->id();

    } else {
        m_processedBgPixmap = m_cachedBgPixmap; // No blur, use original cached
    }
    m_lastBlurState = m_zoneData->blurBackgroundImage();
    update(); // Request repaint as processed image changed
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
