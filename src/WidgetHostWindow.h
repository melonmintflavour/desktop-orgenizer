#ifndef WIDGETHOSTWINDOW_H
#define WIDGETHOSTWINDOW_H

#include <QWidget>
#include <QPoint>

class WidgetHostWindow : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetHostWindow(QWidget *parent = nullptr);
    ~WidgetHostWindow() override;

    // Allow setting a central widget or managing a layout directly
    void setContentWidget(QWidget* content);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    // To allow subclasses to easily make parts non-draggable (e.g. content area)
    virtual bool isPointDraggable(const QPoint& pos);
    virtual void onDragFinished(); // Called when a drag operation concludes

private:
    QPoint m_dragPosition;
    bool m_isDragging;
    QWidget* m_contentWidget; // Optional: if we use a single content widget approach
};

#endif // WIDGETHOSTWINDOW_H
