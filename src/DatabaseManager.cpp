#include "DatabaseManager.h"
#include "PageManager.h"
#include "PageData.h"
#include "ZoneData.h"
#include "IconData.h"

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QUuid> // For string to QUuid conversion and vice-versa

DatabaseManager::DatabaseManager(const QString& dbName, QObject *parent)
    : QObject(parent)
{
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (dataPath.isEmpty()) {
        qFatal("Cannot determine application data location!");
    }
    QDir dir(dataPath);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            qFatal("Failed to create application data directory: %s", qPrintable(dataPath));
        }
    }
    m_dbPath = dataPath + "/" + dbName;
    qDebug() << "Database path set to:" << m_dbPath;

    m_database = QSqlDatabase::addDatabase("QSQLITE", "desktopOverlayConnection"); // Named connection
    m_database.setDatabaseName(m_dbPath);
}

DatabaseManager::~DatabaseManager()
{
    if (m_database.isOpen()) {
        m_database.close();
    }
    QSqlDatabase::removeDatabase("desktopOverlayConnection"); // Remove the named connection
    qDebug() << "DatabaseManager destroyed, connection closed.";
}

bool DatabaseManager::openDatabase()
{
    if (!m_database.isValid()) {
        qWarning() << "Database driver not valid.";
        return false;
    }
    if (m_database.isOpen()) {
        qDebug() << "Database already open.";
        return true;
    }
    if (!m_database.open()) {
        qWarning() << "Failed to open database:" << m_database.lastError().text();
        return false;
    }
    qDebug() << "Database opened successfully:" << m_dbPath;
    return createTablesIfNotExist();
}

void DatabaseManager::closeDatabase()
{
    if (m_database.isOpen()) {
        m_database.close();
        qDebug() << "Database closed.";
    }
}

bool DatabaseManager::createTablesIfNotExist()
{
    if (!m_database.isOpen()) {
        qWarning() << "Database not open, cannot create tables.";
        return false;
    }

    QSqlQuery query(m_database);
    bool success = true;

    // Pages Table
    if (!query.exec("CREATE TABLE IF NOT EXISTS Pages ("
                    "page_id TEXT PRIMARY KEY NOT NULL,"
                    "page_name TEXT NOT NULL,"
                    "page_order INTEGER NOT NULL UNIQUE,"
                    "wallpaper_path TEXT,"
                    "overlay_color TEXT"
                    ");")) {
        qWarning() << "Failed to create Pages table:" << query.lastError().text();
        success = false;
    }

    // Zones Table
    if (!query.exec("CREATE TABLE IF NOT EXISTS Zones ("
                    "zone_id TEXT PRIMARY KEY NOT NULL,"
                    "page_id TEXT NOT NULL,"
                    "zone_title TEXT,"
                    "pos_x REAL NOT NULL,"
                    "pos_y REAL NOT NULL,"
                    "width REAL NOT NULL,"
                    "height REAL NOT NULL,"
                    "bg_color TEXT,"
                    "corner_radius INTEGER DEFAULT 0,"
                    "background_image_path TEXT,"
                    "blur_background_image INTEGER DEFAULT 0,"
                    "FOREIGN KEY(page_id) REFERENCES Pages(page_id) ON DELETE CASCADE"
                    ");")) {
        qWarning() << "Failed to create Zones table:" << query.lastError().text();
        success = false;
    }

    // Icons Table
    if (!query.exec("CREATE TABLE IF NOT EXISTS Icons ("
                    "icon_id TEXT PRIMARY KEY NOT NULL,"
                    "zone_id TEXT NOT NULL,"
                    "file_path TEXT NOT NULL,"
                    "pos_x_in_zone REAL NOT NULL,"
                    "pos_y_in_zone REAL NOT NULL,"
                    "FOREIGN KEY(zone_id) REFERENCES Zones(zone_id) ON DELETE CASCADE"
                    ");")) {
        qWarning() << "Failed to create Icons table:" << query.lastError().text();
        success = false;
    }

    if (success) {
        qDebug() << "Database tables checked/created successfully.";
    }
    return success;
}


// --- Saving Logic ---
bool DatabaseManager::savePages(const QList<PageData*>& pages)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database not open, cannot save pages.";
        return false;
    }

    m_database.transaction(); // Start transaction for batch saving

    // Clear existing data first to handle deletions and reordering correctly
    // ON DELETE CASCADE should handle related zones and icons
    QSqlQuery clearQuery(m_database);
    if (!clearQuery.exec("DELETE FROM Pages")) {
        qWarning() << "Failed to clear Pages table:" << clearQuery.lastError().text();
        m_database.rollback();
        return false;
    }
    // No need to explicitly delete from Zones and Icons due to ON DELETE CASCADE.

    bool all_success = true;
    for (int i = 0; i < pages.count(); ++i) {
        PageData* page = pages.at(i);
        if (!page) continue;

        if (!savePage(page, i)) { // Save page with its order
            all_success = false;
            break;
        }
        for (ZoneData* zone : page->zones()) {
            if (!zone) continue;
            if (!saveZone(zone, page->id())) {
                all_success = false;
                break;
            }
            for (IconData* icon : zone->icons()) {
                if (!icon) continue;
                if (!saveIcon(icon, zone->id())) {
                    all_success = false;
                    break;
                }
            }
            if (!all_success) break;
        }
        if (!all_success) break;
    }

    if (all_success) {
        m_database.commit();
        qDebug() << "All pages and their contents saved successfully.";
        return true;
    } else {
        qWarning() << "Failed to save one or more items. Rolling back transaction.";
        m_database.rollback();
        return false;
    }
}

bool DatabaseManager::savePage(PageData* pageData, int order)
{
    QSqlQuery query(m_database);
    query.prepare("INSERT INTO Pages (page_id, page_name, page_order, wallpaper_path, overlay_color) "
                  "VALUES (:page_id, :page_name, :page_order, :wallpaper_path, :overlay_color)");
    query.bindValue(":page_id", pageData->id().toString());
    query.bindValue(":page_name", pageData->name());
    query.bindValue(":page_order", order);
    query.bindValue(":wallpaper_path", pageData->wallpaperPath().isEmpty() ? QVariant(QVariant::String) : pageData->wallpaperPath());
    query.bindValue(":overlay_color", pageData->overlayColor().isValid() ? pageData->overlayColor().name(QColor::HexArgb) : QVariant(QVariant::String));


    if (!query.exec()) {
        qWarning() << "Failed to save page" << pageData->id() << ":" << query.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::saveZone(ZoneData* zoneData, const QUuid& pageId)
{
    QSqlQuery query(m_database);
    query.prepare("INSERT INTO Zones (zone_id, page_id, zone_title, pos_x, pos_y, width, height, bg_color, corner_radius, background_image_path, blur_background_image) "
                  "VALUES (:zone_id, :page_id, :zone_title, :pos_x, :pos_y, :width, :height, :bg_color, :corner_radius, :bg_image_path, :blur_bg_image)");
    query.bindValue(":zone_id", zoneData->id().toString());
    query.bindValue(":page_id", pageId.toString());
    query.bindValue(":zone_title", zoneData->title());
    query.bindValue(":pos_x", zoneData->geometry().x());
    query.bindValue(":pos_y", zoneData->geometry().y());
    query.bindValue(":width", zoneData->geometry().width());
    query.bindValue(":height", zoneData->geometry().height());
    query.bindValue(":bg_color", zoneData->backgroundColor().name(QColor::HexArgb));
    query.bindValue(":corner_radius", zoneData->cornerRadius());
    query.bindValue(":bg_image_path", zoneData->backgroundImagePath().isEmpty() ? QVariant(QVariant::String) : zoneData->backgroundImagePath()); // Store NULL if empty
    query.bindValue(":blur_bg_image", zoneData->blurBackgroundImage() ? 1 : 0);


    if (!query.exec()) {
        qWarning() << "Failed to save zone" << zoneData->id() << ":" << query.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::saveIcon(IconData* iconData, const QUuid& zoneId)
{
    QSqlQuery query(m_database);
    query.prepare("INSERT INTO Icons (icon_id, zone_id, file_path, pos_x_in_zone, pos_y_in_zone) "
                  "VALUES (:icon_id, :zone_id, :file_path, :pos_x_in_zone, :pos_y_in_zone)");
    query.bindValue(":icon_id", iconData->id().toString());
    query.bindValue(":zone_id", zoneId.toString());
    query.bindValue(":file_path", iconData->filePath());
    query.bindValue(":pos_x_in_zone", iconData->positionInZone().x());
    query.bindValue(":pos_y_in_zone", iconData->positionInZone().y());

    if (!query.exec()) {
        qWarning() << "Failed to save icon" << iconData->id() << ":" << query.lastError().text();
        return false;
    }
    return true;
}


// --- Loading Logic ---
bool DatabaseManager::loadPages(PageManager* pageManager)
{
    if (!m_database.isOpen() || !pageManager) {
        qWarning() << "Database not open or PageManager invalid, cannot load pages.";
        return false;
    }

    pageManager->clearAllPages(); // Clear any existing in-memory pages before loading

    QSqlQuery pageQuery("SELECT page_id, page_name, wallpaper_path, overlay_color FROM Pages ORDER BY page_order ASC", m_database);
    if (!pageQuery.exec()) {
        qWarning() << "Failed to load pages:" << pageQuery.lastError().text();
        return false;
    }

    qDebug() << "Loading pages from database...";
    while (pageQuery.next()) {
        QUuid pageId = QUuid(pageQuery.value("page_id").toString());
        QString pageName = pageQuery.value("page_name").toString();
        QString wallpaperPath = pageQuery.value("wallpaper_path").toString();
        QColor overlayColor(pageQuery.value("overlay_color").toString()); // QColor can parse #AARRGGBB

        PageData* newPageData = new PageData(pageId, pageName);
        newPageData->setWallpaperPath(wallpaperPath);
        newPageData->setOverlayColor(overlayColor.isValid() ? overlayColor : Qt::transparent); // Ensure valid color or default

        // Load Zones for this page
        QSqlQuery zoneQuery(m_database);
        zoneQuery.prepare("SELECT zone_id, zone_title, pos_x, pos_y, width, height, bg_color, corner_radius, background_image_path, blur_background_image "
                          "FROM Zones WHERE page_id = :page_id");
        zoneQuery.bindValue(":page_id", pageId.toString());
        if (!zoneQuery.exec()) {
            qWarning() << "Failed to load zones for page" << pageId << ":" << zoneQuery.lastError().text();
            delete newPageData; // Clean up if loading its contents fails
            continue;
        }

        while (zoneQuery.next()) {
            QUuid zoneId = QUuid(zoneQuery.value("zone_id").toString());
            QString zoneTitle = zoneQuery.value("zone_title").toString();
            QRectF zoneGeo(zoneQuery.value("pos_x").toReal(),
                           zoneQuery.value("pos_y").toReal(),
                           zoneQuery.value("width").toReal(),
                           zoneQuery.value("height").toReal());
            QColor zoneColor(zoneQuery.value("bg_color").toString());
            int cornerRadius = zoneQuery.value("corner_radius").toInt();
            QString bgImagePath = zoneQuery.value("background_image_path").toString();
            bool blurBgImage = zoneQuery.value("blur_background_image").toInt() == 1;

            ZoneData* newZoneData = new ZoneData(zoneId, zoneTitle, zoneGeo, zoneColor, cornerRadius, bgImagePath, blurBgImage);

            // Load Icons for this zone
            QSqlQuery iconQuery(m_database);
            iconQuery.prepare("SELECT icon_id, file_path, pos_x_in_zone, pos_y_in_zone "
                              "FROM Icons WHERE zone_id = :zone_id");
            iconQuery.bindValue(":zone_id", zoneId.toString());
            if (!iconQuery.exec()) {
                qWarning() << "Failed to load icons for zone" << zoneId << ":" << iconQuery.lastError().text();
                delete newZoneData; // Clean up
                continue;
            }
            while (iconQuery.next()) {
                QUuid iconId = QUuid(iconQuery.value("icon_id").toString());
                QString filePath = iconQuery.value("file_path").toString();
                QPointF iconPos(iconQuery.value("pos_x_in_zone").toReal(),
                                iconQuery.value("pos_y_in_zone").toReal());
                IconData* newIconData = new IconData(iconId, filePath, iconPos);
                newZoneData->addIcon(newIconData); // ZoneData takes ownership
            }
            newPageData->addZone(newZoneData); // PageData takes ownership
        }
        pageManager->addLoadedPage(newPageData); // PageManager needs this method
    }

    qDebug() << "Finished loading pages from database. Total pages loaded:" << pageManager->pageCount();
    if (pageManager->pageCount() > 0 && pageManager->activePageIndex() == -1) {
        pageManager->setActivePageIndex(0); // Activate first page if any loaded
    }
    return true;
}
