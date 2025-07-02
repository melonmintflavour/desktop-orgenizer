#include "PageManager.h"
#include "ZoneData.h" // For ZoneData type
#include <QDebug>

PageManager::PageManager(QObject *parent)
    : QObject(parent), m_activePageIndex(-1)
{
}

const QList<PageData*>& PageManager::pages() const
{
    return m_pages;
}

PageData* PageManager::page(int index) const
{
    if (index >= 0 && index < m_pages.size()) {
        return m_pages.at(index);
    }
    return nullptr;
}

PageData* PageManager::pageById(const QUuid& id) const
{
    for (PageData* p : m_pages) {
        if (p->id() == id) {
            return p;
        }
    }
    return nullptr;
}

int PageManager::pageCount() const
{
    return m_pages.size();
}

PageData* PageManager::addPage(const QString& name)
{
    // Check for duplicate names? For now, allow.
    PageData* newPage = new PageData(name);
    m_pages.append(newPage);
    int newIndex = m_pages.size() - 1;
    emit pageAdded(newPage, newIndex);

    if (m_activePageIndex == -1 && !m_pages.isEmpty()) {
        setActivePageIndex(0); // Activate the first page if none was active
    }
    return newPage;
}

bool PageManager::removePage(int index)
{
    if (index >= 0 && index < m_pages.size()) {
        PageData* pageToRemove = m_pages.takeAt(index);
        QUuid removedId = pageToRemove->id();

        // Clean up zones associated with this page
        // ZoneData destructor now handles deleting its IconData, PageData destructor handles its ZoneData.
        // PageManager just needs to delete the PageData object.
        // for (ZoneData* zone : pageToRemove->zones()) {
        //     delete zone; // This will also delete icons within the zone due to ZoneData destructor
        // }
        // pageToRemove->m_zones.clear(); // No longer needed if PageData destructor handles it (which it should)

        delete pageToRemove; // Clean up page memory. Its destructor will handle its contents.
        emit pageRemoved(removedId, index);

        if (m_pages.isEmpty()) {
            m_activePageIndex = -1;
            emit activePageChanged(nullptr, -1);
        } else if (m_activePageIndex >= index) {
            if (m_activePageIndex == index) {
                 setActivePageIndex(qMax(0, index -1));
            } else if (m_activePageIndex > index) {
                m_activePageIndex--;
            }
        }
        return true;
    }
    return false;
}

bool PageManager::removePageById(const QUuid& id)
{
    for (int i = 0; i < m_pages.size(); ++i) {
        if (m_pages.at(i)->id() == id) {
            return removePage(i);
        }
    }
    return false;
}


int PageManager::activePageIndex() const
{
    return m_activePageIndex;
}

PageData* PageManager::activePage() const
{
    return page(m_activePageIndex);
}

void PageManager::setActivePageIndex(int index)
{
    if (index >= -1 && index < m_pages.size()) {
        if (m_activePageIndex != index) {
            m_activePageIndex = index;
            emit activePageChanged(activePage(), m_activePageIndex);
        }
    } else if (m_pages.isEmpty() && index != -1) {
        if (m_activePageIndex != -1) {
             m_activePageIndex = -1;
             emit activePageChanged(nullptr, -1);
        }
    } else if (index != -1) {
        qWarning() << "PageManager::setActivePageIndex: Invalid index" << index << "for page count" << m_pages.size();
    }
}

void PageManager::setActivePageById(const QUuid& id)
{
    for (int i = 0; i < m_pages.size(); ++i) {
        if (m_pages.at(i)->id() == id) {
            setActivePageIndex(i);
            return;
        }
    }
    qWarning() << "PageManager::setActivePageById: Page with ID" << id << "not found.";
}

// --- Zone Management ---

ZoneData* PageManager::addZoneToActivePage(const QString& title, const QRectF& geometry, const QColor& backgroundColor)
{
    PageData* currentPage = activePage();
    if (!currentPage) {
        qWarning() << "PageManager::addZoneToActivePage: No active page to add zone to.";
        return nullptr;
    }
    ZoneData* newZone = new ZoneData(title, geometry, backgroundColor);
    currentPage->addZone(newZone);
    emit zoneAddedToPage(currentPage, newZone);
    return newZone;
}

ZoneData* PageManager::addZoneToPage(const QUuid& pageId, const QString& title, const QRectF& geometry, const QColor& backgroundColor)
{
    PageData* targetPage = pageById(pageId);
    if (!targetPage) {
        qWarning() << "PageManager::addZoneToPage: Page with ID" << pageId << "not found.";
        return nullptr;
    }
    ZoneData* newZone = new ZoneData(title, geometry, backgroundColor);
    targetPage->addZone(newZone);
    emit zoneAddedToPage(targetPage, newZone);
    return newZone;
}

bool PageManager::removeZoneFromActivePage(const QUuid& zoneId)
{
    PageData* currentPage = activePage();
    if (!currentPage) {
        qWarning() << "PageManager::removeZoneFromActivePage: No active page.";
        return false;
    }
    return removeZoneFromPage(currentPage->id(), zoneId);
}

bool PageManager::removeZoneFromPage(const QUuid& pageId, const QUuid& zoneId)
{
    PageData* targetPage = pageById(pageId);
    if (!targetPage) {
        qWarning() << "PageManager::removeZoneFromPage: Page with ID" << pageId << "not found.";
        return false;
    }

    ZoneData* zoneToRemove = targetPage->zoneById(zoneId);
    if (zoneToRemove) {
        // targetPage->removeZoneById will delete the IconData within the ZoneData,
        // and then ZoneData itself if removeZoneById in PageData also deletes it.
        // Or, PageManager can delete zoneToRemove after PageData confirms removal from its list.
        // Current PageData::removeZoneById does NOT delete the ZoneData pointer, just removes from list.
        // So PageManager MUST delete zoneToRemove.
        if (targetPage->removeZoneById(zoneId)) { // This just removes from list
            emit zoneRemovedFromPage(targetPage, zoneId);
            delete zoneToRemove; // PageManager deletes the ZoneData object, which in turn deletes its IconData
            return true;
        }
    }
    qWarning() << "PageManager::removeZoneFromPage: Zone with ID" << zoneId << "not found on page" << pageId;
    return false;
}

void PageManager::updateZoneData(ZoneData* zone)
{
    if (zone) {
        // Potentially validate if the zone belongs to any page managed by this manager, if necessary
        emit zoneDataChanged(zone);
    }
}

bool PageManager::renamePage(const QUuid& pageId, const QString& newName)
{
    PageData* pageToRename = pageById(pageId);
    if (pageToRename) {
        if (pageToRename->name() == newName) {
            return true; // No change needed
        }
        pageToRename->setName(newName);
        emit pageNameChanged(pageToRename);
        qDebug() << "Page" << pageId << "renamed to" << newName;
        return true;
    }
    qWarning() << "PageManager::renamePage: Page with ID" << pageId << "not found.";
    return false;
}

void PageManager::movePage(int fromIndex, int toIndex)
{
    if (fromIndex < 0 || fromIndex >= m_pages.size() ||
        toIndex < 0 || toIndex >= m_pages.size() ||
        fromIndex == toIndex) {
        qWarning() << "PageManager::movePage: Invalid indices provided. From:" << fromIndex << "To:" << toIndex << "Count:" << m_pages.size();
        return;
    }

    PageData* pageToMove = m_pages.takeAt(fromIndex);
    m_pages.insert(toIndex, pageToMove);

    // Adjust active page index if it was affected by the move
    if (m_activePageIndex == fromIndex) {
        m_activePageIndex = toIndex;
    } else if (fromIndex < m_activePageIndex && toIndex >= m_activePageIndex) {
        m_activePageIndex--;
    } else if (fromIndex > m_activePageIndex && toIndex <= m_activePageIndex) {
        m_activePageIndex++;
    }
    // Note: activePageChanged signal is not emitted here as the active page *instance* hasn't changed,
    // only its index relative to other pages. The UI (tabs) will reflect the order change directly.
    // If other parts of the app depend on knowing the active page's *index* has changed due to reorder,
    // then an activePageChanged or a specific pageIndexChanged signal might be needed.

    emit pageOrderChanged();
    qDebug() << "Page moved from index" << fromIndex << "to" << toIndex;
}


// --- PageManager internal methods for DatabaseManager ---
void PageManager::addLoadedPage(PageData* pageData) {
    if (pageData) {
        m_pages.append(pageData);
        // Emitting pageAdded here would cause MainWindow to create a new tab.
        // This is correct behavior when loading.
        emit pageAdded(pageData, m_pages.size() - 1);
    }
}

void PageManager::clearAllPages() {
    // This needs to properly delete all PageData and their owned ZoneData/IconData
    // qDeleteAll uses the delete operator on each pointer in the container and then clears the container.
    qDeleteAll(m_pages);
    m_pages.clear();

    int oldActiveIndex = m_activePageIndex;
    m_activePageIndex = -1;

    if (oldActiveIndex != -1) { // Only emit if it actually changed
        emit activePageChanged(nullptr, -1);
    }
    // emit allPagesCleared(); // Optional signal if other components need to react to full clear
    qDebug() << "All pages cleared from PageManager.";
}

void PageManager::notifyPagePropertiesChanged(PageData* page)
{
    if (page) {
        emit pagePropertiesChanged(page);
        // This also implies that the overall state has changed and might need saving.
        // The main application window (MainWindow) will handle calling saveSettings on close,
        // which iterates through all pages and saves their current state.
        qDebug() << "Page properties changed notification for page:" << page->id();
    }
}
