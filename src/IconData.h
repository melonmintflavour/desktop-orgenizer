#ifndef ICONDATA_H
#define ICONDATA_H

#include <QString>
#include <QPointF>
#include <QUuid>

class IconData
{
public:
    IconData(const QString& filePath, const QPointF& positionInZone)
        : m_id(QUuid::createUuid()), m_filePath(filePath), m_positionInZone(positionInZone)
    {
    }

    IconData(QUuid id, const QString& filePath, const QPointF& positionInZone)
        : m_id(id), m_filePath(filePath), m_positionInZone(positionInZone)
    {
    }

    QUuid id() const { return m_id; }
    QString filePath() const { return m_filePath; }
    QPointF positionInZone() const { return m_positionInZone; }
    QString displayName() const; // Extracts file name from path

    void setFilePath(const QString& filePath) { m_filePath = filePath; }
    void setPositionInZone(const QPointF& pos) { m_positionInZone = pos; }
    // void setCachedIcon(const QPixmap& icon); // For later
    // QPixmap cachedIcon() const; // For later

private:
    QUuid m_id;
    QString m_filePath;
    QPointF m_positionInZone; // Relative to its parent ZoneWidget
    // QPixmap m_cachedIcon; // For later optimization
};

#endif // ICONDATA_H
