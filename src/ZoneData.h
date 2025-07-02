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
    ZoneData(const QString& title, const QRectF& geometry, const QColor& backgroundColor); // For new zones
    // Main constructor for loading from DB
    ZoneData(QUuid id, const QString& title, const QRectF& geometry, const QColor& backgroundColor,
             int cornerRadius, const QString& bgImagePath, bool blurBgImage);
    ~ZoneData(); // Destructor to clean up IconData objects

    QUuid id() const { return m_id; }
    QString title() const { return m_title; }
    QRectF geometry() const { return m_geometry; }
    QColor backgroundColor() const { return m_backgroundColor; }
    int cornerRadius() const { return m_cornerRadius; }
    QString backgroundImagePath() const { return m_backgroundImagePath; }
    bool blurBackgroundImage() const { return m_blurBackgroundImage; }

    void setTitle(const QString& title) { m_title = title; }
    void setGeometry(const QRectF& geometry) { m_geometry = geometry; }
    void setBackgroundColor(const QColor& color) { m_backgroundColor = color; }
    void setCornerRadius(int radius) { m_cornerRadius = qMax(0, radius); } // Ensure non-negative
    void setBackgroundImagePath(const QString& path) { m_backgroundImagePath = path; }
    void setBlurBackgroundImage(bool blur) { m_blurBackgroundImage = blur; }

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
    int m_cornerRadius; // For rounded corners
    QString m_backgroundImagePath;
    bool m_blurBackgroundImage;
    QList<IconData*> m_icons; // List of icons in this zone
};

#endif // ZONEDATA_H
