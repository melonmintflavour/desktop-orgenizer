#include "PageData.h"
#include "ZoneData.h" // Required for ZoneData definition
#include <QDebug>
#include <QColor> // For Qt::transparent

PageData::PageData(const QString& name)
    : m_id(QUuid::createUuid()), m_name(name), m_overlayColor(Qt::transparent) // Initialize new members
{
    qDebug() << "PageData created (new UUID):" << m_id << name;
}

PageData::PageData(QUuid id, const QString& name)
    : m_id(id), m_name(name), m_overlayColor(Qt::transparent) // Initialize new members
{
    qDebug() << "PageData created (existing UUID):" << m_id << name;
}

// Constructor for loading from DB will be updated in DatabaseManager to pass these
// For now, ensure existing constructors initialize them.

PageData::~PageData()
{
    qDebug() << "PageData destroyed:" << m_id << m_name << "Clearing" << m_zones.count() << "zones.";
    // Delete all ZoneData objects this page owns
    qDeleteAll(m_zones);
    m_zones.clear();
}


// Implementation for PageData methods related to zones

ZoneData* PageData::zone(int index) const
{
    if (index >= 0 && index < m_zones.size()) {
        return m_zones.at(index);
    }
    return nullptr;
}

ZoneData* PageData::zoneById(const QUuid& id) const
{
    for (ZoneData* z : m_zones) {
        if (z && z->id() == id) {
            return z;
        }
    }
    return nullptr;
}

void PageData::addZone(ZoneData* zone)
{
    if (zone && !m_zones.contains(zone)) {
        m_zones.append(zone);
        qDebug() << "Zone" << zone->id() << "added to page" << m_id;
    }
}

bool PageData::removeZone(ZoneData* zone)
{
    if (!zone) return false;
    bool removed = m_zones.removeOne(zone);
    if (removed) {
        qDebug() << "Zone" << zone->id() << "removed from page" << m_id << "(pointer match)";
        // Caller (PageManager) is responsible for deleting the zone object itself
    }
    return removed;
}

bool PageData::removeZoneById(const QUuid& id)
{
    for (int i = 0; i < m_zones.size(); ++i) {
        if (m_zones.at(i) && m_zones.at(i)->id() == id) {
            ZoneData* zoneToRemove = m_zones.takeAt(i);
            // Caller (PageManager) is responsible for deleting zoneToRemove.
            qDebug() << "Zone" << id << "removed from page" << m_id << "(ID match)";
            return true;
        }
    }
    qWarning() << "Zone" << id << "not found on page" << m_id << "for removal by ID.";
    return false;
}
