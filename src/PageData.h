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
    // QList<ZoneData*> m_zones; // Moved to private

public: // Accessors for zones
    const QList<ZoneData*>& zones() const { return m_zones; } // Will access private m_zones
    ZoneData* zone(int index) const;
    ZoneData* zoneById(const QUuid& id) const;
    void addZone(ZoneData* zone);
    bool removeZone(ZoneData* zone); // Returns true if found and removed
    bool removeZoneById(const QUuid& id);

    // Wallpaper and Overlay
    QString wallpaperPath() const { return m_wallpaperPath; }
    void setWallpaperPath(const QString& path) { m_wallpaperPath = path; }
    QColor overlayColor() const { return m_overlayColor; }
    void setOverlayColor(const QColor& color) { m_overlayColor = color; }


private:
    QUuid m_id;
    QString m_name;
    QList<ZoneData*> m_zones; // Correctly private now
    QString m_wallpaperPath;
    QColor m_overlayColor;
};

#endif // PAGEDATA_H
