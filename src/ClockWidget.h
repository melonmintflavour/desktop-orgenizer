#ifndef CLOCKWIDGET_H
#define CLOCKWIDGET_H

#include <QWidget>

class QLabel;
class QTimer;

class ClockWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ClockWidget(QWidget *parent = nullptr);
    ~ClockWidget() override;

private slots:
    void updateDisplay();

private:
    void setupUI();

    QLabel* m_timeLabel;
    QLabel* m_dateLabel;
    QTimer* m_timer;
};

#endif // CLOCKWIDGET_H
