#ifndef ICONWIDGET_H
#define ICONWIDGET_H

#include <QWidget>
#include <QPointF>
#include <QMenu>

class IconData;   // Forward declaration
class ZoneWidget; // Forward declaration (parent)
class PageManager; // Forward declaration (for notifying changes indirectly)

class IconWidget : public QWidget
{
    Q_OBJECT

public:
    explicit IconWidget(IconData* iconData, PageManager* pageManager, ZoneWidget *parentZoneWidget);
    ~IconWidget() override;

    IconData* data() const { return m_iconData; }
    void updateFromData(); // Update widget appearance/position from m_iconData

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override; // For launching
    void contextMenuEvent(QContextMenuEvent *event) override;


private slots:
    void removeIconRequested();
    // void launchFileRequested(); // For later

private:
    IconData* m_iconData;
    PageManager* m_pageManager; // To notify of changes that need saving (via ZoneData)
    ZoneWidget* m_parentZoneWidget; // To access parent zone's data/methods if needed

    bool m_isDragging;
    QPoint m_dragStartPosition; // Relative to widget's top-left, for dragging
};

#endif // ICONWIDGET_H
