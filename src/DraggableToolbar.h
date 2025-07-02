#ifndef DRAGGABLETOOLBAR_H
#define DRAGGABLETOOLBAR_H

#include "WidgetHostWindow.h"
#include <QBoxLayout>

class DraggableToolbar : public WidgetHostWindow
{
    Q_OBJECT
public:
    enum class Orientation { Horizontal, Vertical };

    explicit DraggableToolbar(Orientation orientation = Orientation::Horizontal, QWidget *parent = nullptr);
    ~DraggableToolbar() override;

    void addWidget(QWidget* widget);
    void addAction(QAction* action); // Will create a QToolButton
    void addSeparator();

protected:
    // void mouseReleaseEvent(QMouseEvent *event) override; // Now handled by onDragFinished
    void onDragFinished() override;
    // bool isPointDraggable(const QPoint& pos) override; // Example: make only specific parts draggable

private:
    void snapToScreenEdge();

    QBoxLayout* m_toolbarLayout;
    Orientation m_orientation;

    static const int SNAP_THRESHOLD = 20; // Pixels from edge to snap
};

#endif // DRAGGABLETOOLBAR_H
