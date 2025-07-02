#ifndef ZONEWIDGET_H
#define ZONEWIDGET_H

#include <QWidget>
#include <QString>
#include <QRectF>
#include <QColor>
#include <QPoint>
#include <QMenu>
#include <QList> // For IconWidgets
#include <QMimeData> // For drag and drop

class ZoneData;    // Forward declaration
class PageManager; // Forward declaration for signaling updates
class IconWidget;  // Forward declaration
class IconData;    // Forward declaration

class ZoneWidget : public QWidget
{
    Q_OBJECT

public:
    enum class ResizeRegion {
        None, Top, Bottom, Left, Right, TopLeft, TopRight, BottomLeft, BottomRight, Move
    };
    Q_ENUM(ResizeRegion)

    explicit ZoneWidget(ZoneData* zoneData, PageManager* pageManager, QWidget *parent = nullptr);
    ~ZoneWidget() override;

    ZoneData* data() const { return m_zoneData; }
    void updateFromData(); // Update widget appearance AND icons from m_zoneData
    void filterIcons(const QString& filterText); // New method for icon filtering

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override; // For title editing
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;


private slots:
    void removeZoneRequested();
    void renameZoneRequested();
    void changeBackgroundColorRequested();
    void setCornerRadiusRequested();
    void setBackgroundImageRequested();
    void clearBackgroundImageRequested();
    void toggleBlurBackgroundImageRequested();

private:
    void updateCursorShape(const QPoint& pos);
    ResizeRegion getResizeRegion(const QPoint& pos);
    void handleResize(const QPoint& newMousePos);
    void handleMove(const QPoint& newMousePos);
    void loadOrUpdateIcons(); // Helper to create/update IconWidgets
    IconWidget* findIconWidget(const QUuid& iconId);
    void loadBackgroundImage(); // Helper to load/cache bg image
    void prepareProcessedBackgroundImage(); // Applies blur if needed


    ZoneData* m_zoneData;
    PageManager* m_pageManager; // To notify of changes that need saving
    QList<IconWidget*> m_iconWidgets; // Keep track of icon widgets
    QPixmap m_cachedBgPixmap;      // Cache for the background image
    QString m_loadedBgImagePath;   // Path of the currently loaded m_cachedBgPixmap
    QPixmap m_processedBgPixmap;   // Potentially blurred/tinted version for painting
    bool m_lastBlurState;          // To detect change in blur state

    bool m_isResizing;
    bool m_isMoving;
    QPoint m_dragStartPosition;  // Relative to widget's top-left
    QPoint m_mousePressPosition; // Global position at mouse press
    QRectF m_originalGeometry;   // Geometry at start of resize/move
    ResizeRegion m_currentResizeRegion;

    static const int RESIZE_BORDER_SENSITIVITY = 10; // Pixels for resize handles
};

#endif // ZONEWIDGET_H
