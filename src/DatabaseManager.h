#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QString>
#include <QtSql/QSqlDatabase>
#include <QList>

class PageData;   // Forward declaration
class ZoneData;   // Forward declaration
class IconData;   // Forward declaration
class PageManager; // Forward declaration

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseManager(const QString& dbPath, QObject *parent = nullptr);
    ~DatabaseManager();

    bool openDatabase();
    void closeDatabase();

    bool loadPages(PageManager* pageManager); // Populates PageManager from DB
    bool savePages(const QList<PageData*>& pages); // Saves all pages and their contents

private:
    bool createTablesIfNotExist();

    // Helper save methods
    bool savePage(PageData* pageData, int order);
    bool saveZone(ZoneData* zoneData, const QUuid& pageId);
    bool saveIcon(IconData* iconData, const QUuid& zoneId);

    // Helper load methods
    // Load methods will directly populate PageData, ZoneData, IconData objects

    QString m_dbPath;
    QSqlDatabase m_database;
};

#endif // DATABASEMANAGER_H
