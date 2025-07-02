#include "ClockWidget.h"
#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>
#include <QTime>
#include <QDate>
#include <QDebug>

ClockWidget::ClockWidget(QWidget *parent)
    : QWidget(parent), m_timer(new QTimer(this))
{
    setObjectName("ClockWidget");
    setupUI();

    connect(m_timer, &QTimer::timeout, this, &ClockWidget::updateDisplay);
    m_timer->start(1000); // Update every second

    updateDisplay(); // Initial display
}

ClockWidget::~ClockWidget()
{
    qDebug() << "ClockWidget destroyed";
}

void ClockWidget::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(5,5,5,5);
    layout->setSpacing(2);

    m_timeLabel = new QLabel(this);
    m_timeLabel->setObjectName("ClockWidget_TimeLabel");
    m_timeLabel->setAlignment(Qt::AlignCenter);
    // m_timeLabel->setStyleSheet("font-size: 18pt; font-weight: bold; color: #E0E0E0;"); // Example, use theme

    m_dateLabel = new QLabel(this);
    m_dateLabel->setObjectName("ClockWidget_DateLabel");
    m_dateLabel->setAlignment(Qt::AlignCenter);
    // m_dateLabel->setStyleSheet("font-size: 10pt; color: #B0B0B0;"); // Example, use theme

    layout->addWidget(m_timeLabel);
    layout->addWidget(m_dateLabel);
    setLayout(layout);

    // Adjust size hint based on typical content
    // This is a rough estimate. Dynamic sizing or fixed size might be better.
    QFontMetrics timeMetrics(m_timeLabel->font());
    // int timeWidth = timeMetrics.horizontalAdvance("00:00:00 WW"); // "WW" for AM/PM if used
    // QFontMetrics dateMetrics(m_dateLabel->font());
    // int dateWidth = dateMetrics.horizontalAdvance("Wednesday, 30. September 2020");
    // setMinimumSize(qMax(timeWidth, dateWidth) + 20, timeMetrics.height() + dateMetrics.height() + 20);
    setMinimumSize(150, 50); // A reasonable default minimum
    adjustSize();

}

void ClockWidget::updateDisplay()
{
    QTime currentTime = QTime::currentTime();
    QDate currentDate = QDate::currentDate();

    // More flexible time format, could be user-configurable later
    // Or use QLocale::system().timeFormat(QLocale::ShortFormat)
    m_timeLabel->setText(currentTime.toString("hh:mm:ss AP"));
    m_dateLabel->setText(currentDate.toString(Qt::DefaultLocaleLongDate));
}
