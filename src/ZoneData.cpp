#include "ZoneData.h"
#include "IconData.h" // Required for IconData definition
#include <QDebug>

ZoneData::ZoneData(const QString& title, const QRectF& geometry, const QColor& backgroundColor)
    : m_id(QUuid::createUuid()), m_title(title), m_geometry(geometry), m_backgroundColor(backgroundColor)
{
    qDebug() << "ZoneData created (new UUID):" << m_id << title;
}

ZoneData::ZoneData(QUuid id, const QString& title, const QRectF& geometry, const QColor& backgroundColor)
    : m_id(id), m_title(title), m_geometry(geometry), m_backgroundColor(backgroundColor)
{
    qDebug() << "ZoneData created (existing UUID):" << m_id << title;
}

ZoneData::~ZoneData()
{
    qDebug() << "ZoneData destroyed:" << m_id << m_title << "Clearing" << m_icons.count() << "icons.";
    // Delete all IconData objects this zone owns
    qDeleteAll(m_icons);
    m_icons.clear();
}

void ZoneData::addIcon(IconData* icon)
{
    if (icon && !m_icons.contains(icon)) {
        m_icons.append(icon);
        qDebug() << "Icon" << icon->id() << "added to zone" << m_id;
    }
}

bool ZoneData::removeIcon(const QUuid& iconId)
{
    for (int i = 0; i < m_icons.size(); ++i) {
        if (m_icons.at(i) && m_icons.at(i)->id() == iconId) {
            IconData* iconToRemove = m_icons.takeAt(i);
            delete iconToRemove; // ZoneData owns its IconData objects
            qDebug() << "Icon" << iconId << "removed from zone" << m_id;
            return true;
        }
    }
    qWarning() << "Icon" << iconId << "not found in zone" << m_id << "for removal.";
    return false;
}

IconData* ZoneData::findIcon(const QUuid& iconId) const
{
    for (IconData* icon : m_icons) {
        if (icon && icon->id() == iconId) {
            return icon;
        }
    }
    return nullptr;
}
