#ifndef TODODATA_H
#define TODODATA_H

#include <QUuid>
#include <QString>
#include <QDateTime>

struct TodoItem {
    QUuid id;
    QString description;
    bool isCompleted;
    QDateTime createdAt;
    QDateTime completedAt; // Valid only if isCompleted is true

    TodoItem() : id(QUuid::createUuid()), isCompleted(false), createdAt(QDateTime::currentDateTime()) {}

    TodoItem(QString desc)
        : id(QUuid::createUuid()), description(desc), isCompleted(false), createdAt(QDateTime::currentDateTime())
    {}

    // For loading from persistence
    TodoItem(QUuid anId, QString desc, bool completed, QDateTime created, QDateTime completedTime)
        : id(anId), description(desc), isCompleted(completed), createdAt(created), completedAt(completedTime)
    {}
};

#endif // TODODATA_H
