#include "WidgetHostWindow.h"
#include <QMouseEvent>
#include <QVBoxLayout> // Example layout
#include <QApplication> // For screen geometry if needed later
#include <QScreen>      // For screen geometry if needed later
#include <QDebug>
#include <QGraphicsDropShadowEffect> // For drop shadow

WidgetHostWindow::WidgetHostWindow(QWidget *parent)
    : QWidget(parent, Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint),
      m_isDragging(false), m_contentWidget(nullptr)
{
    setAttribute(Qt::WA_TranslucentBackground); // Required for shadow to look right if window itself has a fill

    // Add drop shadow effect
    QGraphicsDropShadowEffect* shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setBlurRadius(18);
    shadowEffect->setColor(QColor(0, 0, 0, 80));
    shadowEffect->setOffset(3, 3);
    setGraphicsEffect(shadowEffect);

    // It's often good for these kinds of windows to delete themselves when closed.
    // setAttribute(Qt::WA_DeleteOnClose); // Can be set by user of this class if desired

    // Basic size, will likely be adjusted by content or subclass
    resize(200, 100);
}

WidgetHostWindow::~WidgetHostWindow()
{
    qDebug() << "WidgetHostWindow destroyed.";
}

void WidgetHostWindow::setContentWidget(QWidget* content)
{
    if (m_contentWidget && m_contentWidget->layout() == layout()) {
        // If the old content widget was using the host's main layout
        delete layout(); // Remove the old layout
    } else if (m_contentWidget) {
        m_contentWidget->setParent(nullptr); // Unparent if it wasn't in main layout
        delete m_contentWidget; // Or just hide, depending on ownership model
    }

    m_contentWidget = content;
    if (m_contentWidget) {
        // Ensure the content widget is parented to this host window
        m_contentWidget->setParent(this);

        // Option 1: Simple central widget approach (no explicit layout in host)
        // m_contentWidget->setGeometry(this->rect()); // Make it fill the host
        // m_contentWidget->show();
        // In this case, resizing the host should resize the content.

        // Option 2: Use a layout in the host window to manage the content widget
        QVBoxLayout* mainLayout = new QVBoxLayout(this); // Or other layout type
        mainLayout->addWidget(m_contentWidget);
        mainLayout->setContentsMargins(0,0,0,0); // No margins for the content
        setLayout(mainLayout);

        adjustSize(); // Adjust host size to content's sizeHint if layout is used
    }
}


void WidgetHostWindow::mousePressEvent(QMouseEvent *event)
{
    if (isPointDraggable(event->position().toPoint()) && event->button() == Qt::LeftButton) {
        m_isDragging = true;
        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    } else {
        // event->ignore(); // Let child widgets handle it if not draggable point
        QWidget::mousePressEvent(event);
    }
}

void WidgetHostWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isDragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPosition().toPoint() - m_dragPosition);
        event->accept();
    } else {
        QWidget::mouseMoveEvent(event);
    }
}

void WidgetHostWindow::mouseReleaseEvent(QMouseEvent *event)
{
    bool wasDragging = m_isDragging; // Capture state before it's reset by base or this logic
    m_isDragging = false; // Reset dragging state for this window

    if (wasDragging && event->button() == Qt::LeftButton) {
        onDragFinished(); // Call the virtual method for subclasses
        event->accept();
    } else {
        // If not handling a drag release, pass to base or ignore if already accepted by child
        if(!event->isAccepted()) {
            QWidget::mouseReleaseEvent(event);
        }
    }
}

// Default implementation for onDragFinished
void WidgetHostWindow::onDragFinished() {
    // Base class does nothing by default when drag finishes.
    // Subclasses like DraggableToolbar will override this for snapping.
    qDebug() << "WidgetHostWindow: Drag finished.";
}


bool WidgetHostWindow::isPointDraggable(const QPoint& pos)
{
    // By default, the entire window is draggable if no child widget at 'pos' accepts the event.
    // Qt's event propagation usually handles this: if a child accepts the press, this won't be called.
    // However, explicitly checking can be useful.
    QWidget* child = childAt(pos);
    if (child && child != m_contentWidget && child->testAttribute(Qt::WA_TransparentForMouseEvents)) {
         // If child is transparent for mouse events, consider it draggable
    } else if (child && child != m_contentWidget) {
        return false; // Click was on a child that's not the main content background (e.g. a button)
    }
    // If click is on m_contentWidget's background (not its children), allow drag.
    // This requires m_contentWidget to not absorb all mouse events on its background.
    // Or, more simply, if no interactive child is at pos, then drag.

    // A simpler default: if no child at pos, or if the child is the main content widget itself
    // (and not one of its interactive children), then allow drag.
    if (child == this || child == m_contentWidget) return true; // Click on host background or content background

    // If there's a child, and it's not the content widget itself, assume it's an interactive element
    // This logic is a bit basic. A more robust way is to have a dedicated drag handle area,
    // or rely on Qt's event system where children that want clicks will accept them.
    return (child == nullptr); // Simplest: only drag if clicking on empty area of host
}
