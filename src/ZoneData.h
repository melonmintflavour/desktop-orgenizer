#ifndef ZONEDATA_H
#define ZONEDATA_H

#include <QString>
#include <QRectF>
#include <QColor>
#include <QUuid>
#include <QList>

// Forward declaration for IconData - will be used later
// struct IconData;
class IconData; // Forward declaration

class ZoneData
{
public:
    ZoneData(const QString& title, const QRectF& geometry, const QColor& backgroundColor);
    ZoneData(QUuid id, const QString& title, const QRectF& geometry, const QColor& backgroundColor);
    ~ZoneData(); // Destructor to clean up IconData objects

    QUuid id() const { return m_id; }
    QString title() const { return m_title; }
    QRectF geometry() const { return m_geometry; }
    QColor backgroundColor() const { return m_backgroundColor; }

    void setTitle(const QString& title) { m_title = title; }
    void setGeometry(const QRectF& geometry) { m_geometry = geometry; }
    void setBackgroundColor(const QColor& color) { m_backgroundColor = color; }

    const QList<IconData*>& icons() const { return m_icons; }
    void addIcon(IconData* icon);
    bool removeIcon(const QUuid& iconId);
    IconData* findIcon(const QUuid& iconId) const;


private:
    friend class PageManager; // To allow PageManager to clear m_icons on page deletion more directly if needed

    QUuid m_id;
    QString m_title;
    QRectF m_geometry;
    QColor m_backgroundColor;
    QList<IconData*> m_icons; // List of icons in this zone
};

#endif // ZONEDATA_H
