#ifndef PAGEDATA_H
#define PAGEDATA_H

#include <QString>
#include <QList>
#include <QUuid> // For unique IDs

// Forward declaration for IconData if it were to be included here
// struct IconData;
class ZoneData; // Forward declaration

class PageData
{
public:
    PageData(const QString& name); // Constructor
    PageData(QUuid id, const QString& name); // Constructor with existing ID
    ~PageData(); // Destructor to clean up ZoneData

    QUuid id() const { return m_id; }
    QString name() const { return m_name; }
    void setName(const QString& name) { m_name = name; }

    // In the future, this will hold icons, zones, etc.
    // QList<IconData> icons;
    QList<ZoneData*> m_zones;

public: // Accessors for zones
    const QList<ZoneData*>& zones() const { return m_zones; }
    ZoneData* zone(int index) const;
    ZoneData* zoneById(const QUuid& id) const;
    void addZone(ZoneData* zone);
    bool removeZone(ZoneData* zone); // Returns true if found and removed
    bool removeZoneById(const QUuid& id);


private:
    QUuid m_id;
    QString m_name;
};

#endif // PAGEDATA_H
