#include "DraggableToolbar.h"
#include <QMouseEvent>
#include <QGuiApplication>
#include <QScreen>
#include <QToolButton>
#include <QStyle> // For standard icons or pixel metrics
#include <QDebug>

DraggableToolbar::DraggableToolbar(Orientation orientation, QWidget *parent)
    : WidgetHostWindow(parent), m_orientation(orientation)
{
    if (m_orientation == Orientation::Horizontal) {
        m_toolbarLayout = new QHBoxLayout(this);
        setFixedHeight(50); // Example fixed height for horizontal toolbar
        // Width will be based on content or initial set
    } else {
        m_toolbarLayout = new QVBoxLayout(this);
        setFixedWidth(50); // Example fixed width for vertical toolbar
        // Height will be based on content
    }
    m_toolbarLayout->setContentsMargins(2, 2, 2, 2);
    m_toolbarLayout->setSpacing(2);
    setLayout(m_toolbarLayout);

    // Example: Set a background color for visibility, will be part of theme later
    // setStyleSheet("background-color: rgba(50,50,50,180); border-radius: 3px;");
    setObjectName("DraggableToolbar"); // For stylesheet targeting
}

DraggableToolbar::~DraggableToolbar()
{
    qDebug() << "DraggableToolbar destroyed";
}

void DraggableToolbar::addWidget(QWidget* widget)
{
    if (widget) {
        m_toolbarLayout->addWidget(widget);
    }
}

void DraggableToolbar::addAction(QAction* action)
{
    if (action) {
        QToolButton *button = new QToolButton(this);
        button->setDefaultAction(action);
        button->setFocusPolicy(Qt::NoFocus); // Important for toolbars
        // button->setAutoRaise(true); // Common style for toolbuttons
        m_toolbarLayout->addWidget(button);
    }
}

void DraggableToolbar::addSeparator()
{
    // Simple separator for QHBoxLayout
    if (m_orientation == Orientation::Horizontal) {
        QFrame* line = new QFrame(this);
        line->setFrameShape(QFrame::VLine);
        line->setFrameShadow(QFrame::Sunken);
        m_toolbarLayout->addWidget(line);
    } else { // QVBoxLayout
        QFrame* line = new QFrame(this);
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        m_toolbarLayout->addWidget(line);
    }
}

void DraggableToolbar::onDragFinished()
{
    WidgetHostWindow::onDragFinished(); // Call base though it does nothing currently
    snapToScreenEdge();
}

/*
// Example: Only allow dragging from specific parts of the toolbar, not buttons.
bool DraggableToolbar::isPointDraggable(const QPoint& pos) {
    // If the click is on a child widget (like a QToolButton), don't drag the toolbar.
    QWidget* child = childAt(pos);
    if (child && child != this) { // 'this' refers to DraggableToolbar itself
        return false;
    }
    return true; // Otherwise, allow dragging from empty toolbar space
}
*/

void DraggableToolbar::snapToScreenEdge()
{
    QScreen* screen = QGuiApplication::screenAt(pos());
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    if (!screen) return;

    QRect screenGeometry = screen->availableGeometry(); // Use available to avoid OS taskbars etc.
    QRect windowGeometry = frameGeometry(); // Current window geometry

    int newX = windowGeometry.x();
    int newY = windowGeometry.y();
    bool moved = false;

    // Snap to left edge
    if (qAbs(windowGeometry.left() - screenGeometry.left()) < SNAP_THRESHOLD) {
        newX = screenGeometry.left();
        moved = true;
    }
    // Snap to right edge
    else if (qAbs(windowGeometry.right() - screenGeometry.right()) < SNAP_THRESHOLD) {
        newX = screenGeometry.right() - windowGeometry.width();
        moved = true;
    }

    // Snap to top edge
    if (qAbs(windowGeometry.top() - screenGeometry.top()) < SNAP_THRESHOLD) {
        newY = screenGeometry.top();
        moved = true;
    }
    // Snap to bottom edge
    else if (qAbs(windowGeometry.bottom() - screenGeometry.bottom()) < SNAP_THRESHOLD) {
        newY = screenGeometry.bottom() - windowGeometry.height();
        moved = true;
    }

    if (moved) {
        qDebug() << "Snapping toolbar from" << windowGeometry.topLeft() << "to" << QPoint(newX, newY);
        move(newX, newY);
        // TODO: Persist new position (this will be handled by MainWindow or a dedicated manager)
        // For example, emit a signal: emit geometryChanged(this->geometry());
    }
}
