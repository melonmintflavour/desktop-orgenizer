#include "TodoWidget.h"
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidgetItem>
#include <QSettings>
#include <QMessageBox> // For confirmations
#include <QCloseEvent>
#include <QDebug>

const QString TODO_SETTINGS_GROUP = "TodoWidget";
const QString TASKS_ARRAY_KEY = "tasks";

TodoWidget::TodoWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("TodoWidget");
    setupUI();
    loadTasks(); // Load tasks when widget is created
}

TodoWidget::~TodoWidget()
{
    // saveTasks(); // Save on destruction - though closeEvent is better for hosted widgets
    qDebug() << "TodoWidget destroyed";
}

void TodoWidget::closeEvent(QCloseEvent *event)
{
    saveTasks();
    QWidget::closeEvent(event);
}


void TodoWidget::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Input area
    QHBoxLayout* inputLayout = new QHBoxLayout();
    m_taskInputLineEdit = new QLineEdit(this);
    m_taskInputLineEdit->setPlaceholderText("Enter new task...");
    connect(m_taskInputLineEdit, &QLineEdit::returnPressed, this, &TodoWidget::handleAddTask); // Add on Enter
    inputLayout->addWidget(m_taskInputLineEdit, 1);

    m_addTaskButton = new QPushButton("Add", this);
    connect(m_addTaskButton, &QPushButton::clicked, this, &TodoWidget::handleAddTask);
    inputLayout->addWidget(m_addTaskButton);
    mainLayout->addLayout(inputLayout);

    // Task list
    m_taskListWidget = new QListWidget(this);
    m_taskListWidget->setObjectName("taskListWidget");
    connect(m_taskListWidget, &QListWidget::itemChanged, this, &TodoWidget::handleTaskItemChanged);
    mainLayout->addWidget(m_taskListWidget, 1); // List takes most space

    // Action buttons
    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    m_removeTaskButton = new QPushButton("Remove Selected", this);
    connect(m_removeTaskButton, &QPushButton::clicked, this, &TodoWidget::handleRemoveTask);
    buttonsLayout->addWidget(m_removeTaskButton);

    m_clearCompletedButton = new QPushButton("Clear Completed", this);
    connect(m_clearCompletedButton, &QPushButton::clicked, this, &TodoWidget::handleClearCompletedTasks);
    buttonsLayout->addWidget(m_clearCompletedButton);
    buttonsLayout->addStretch(); // Push buttons to left

    mainLayout->addLayout(buttonsLayout);
    setLayout(mainLayout);
    setMinimumSize(250, 300);
}

void TodoWidget::handleAddTask()
{
    QString description = m_taskInputLineEdit->text().trimmed();
    if (description.isEmpty()) {
        return;
    }
    TodoItem newItem(description);
    m_tasks.append(newItem);

    QListWidgetItem* listItem = new QListWidgetItem(newItem.description, m_taskListWidget);
    listItem->setFlags(listItem->flags() | Qt::ItemIsUserCheckable);
    listItem->setCheckState(Qt::Unchecked);
    listItem->setData(Qt::UserRole, newItem.id); // Store ID
    m_taskListWidget->addItem(listItem);

    m_taskInputLineEdit->clear();
    saveTasks(); // Save after adding
    qDebug() << "Task added:" << newItem.description << newItem.id;
}

void TodoWidget::handleRemoveTask()
{
    QList<QListWidgetItem*> selectedItems = m_taskListWidget->selectedItems();
    if (selectedItems.isEmpty()) {
        if (m_taskListWidget->currentItem()) { // If no multiple selection, use current
            selectedItems.append(m_taskListWidget->currentItem());
        } else {
            return;
        }
    }

    // Confirmation for multiple items
    if (selectedItems.count() > 1) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Confirm Remove",
                                      QString("Remove %1 selected tasks?").arg(selectedItems.count()),
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::No) return;
    }

    for (QListWidgetItem* item : selectedItems) {
        QUuid id = item->data(Qt::UserRole).toUuid();
        for (int i = 0; i < m_tasks.size(); ++i) {
            if (m_tasks.at(i).id == id) {
                m_tasks.removeAt(i);
                break;
            }
        }
        delete m_taskListWidget->takeItem(m_taskListWidget->row(item)); // Remove from UI
        qDebug() << "Task removed:" << id;
    }
    saveTasks();
}

void TodoWidget::handleClearCompletedTasks()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirm Clear",
                                    "Clear all completed tasks?",
                                    QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::No) return;

    bool changed = false;
    for (int i = m_tasks.size() - 1; i >= 0; --i) {
        if (m_tasks.at(i).isCompleted) {
            QUuid idToRemove = m_tasks.at(i).id;
            m_tasks.removeAt(i);
            QListWidgetItem* listItem = findListWidgetItemById(idToRemove);
            if (listItem) {
                delete m_taskListWidget->takeItem(m_taskListWidget->row(listItem));
            }
            changed = true;
            qDebug() << "Completed task cleared:" << idToRemove;
        }
    }
    if (changed) saveTasks();
}

void TodoWidget::handleTaskItemChanged(QListWidgetItem *item)
{
    if (!item) return;
    QUuid id = item->data(Qt::UserRole).toUuid();
    TodoItem* task = findTaskById(id);
    if (task) {
        bool wasCompleted = task->isCompleted;
        task->isCompleted = (item->checkState() == Qt::Checked);
        if (task->isCompleted && !wasCompleted) { // Just completed
            task->completedAt = QDateTime::currentDateTime();
        } else if (!task->isCompleted && wasCompleted) { // Unchecked
             task->completedAt = QDateTime(); // Invalidate completed time
        }
        // Apply strikethrough or other visual indication
        QFont font = item->font();
        font.setStrikeOut(task->isCompleted);
        item->setFont(font);

        saveTasks(); // Save on change
        qDebug() << "Task" << id << "completion state changed to" << task->isCompleted;
    }
}

TodoItem* TodoWidget::findTaskById(const QUuid& id) {
    for (int i = 0; i < m_tasks.size(); ++i) {
        if (m_tasks[i].id == id) {
            return &m_tasks[i];
        }
    }
    return nullptr;
}

QListWidgetItem* TodoWidget::findListWidgetItemById(const QUuid& id) {
    for (int i = 0; i < m_taskListWidget->count(); ++i) {
        QListWidgetItem* item = m_taskListWidget->item(i);
        if (item && item->data(Qt::UserRole).toUuid() == id) {
            return item;
        }
    }
    return nullptr;
}


void TodoWidget::loadTasks()
{
    m_tasks.clear();
    QSettings settings;
    int size = settings.beginReadArray(TASKS_ARRAY_KEY);
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        TodoItem task;
        task.id = settings.value("id").toUuid();
        task.description = settings.value("description").toString();
        task.isCompleted = settings.value("isCompleted").toBool();
        task.createdAt = settings.value("createdAt").toDateTime();
        task.completedAt = settings.value("completedAt").toDateTime();
        m_tasks.append(task);
    }
    settings.endArray();
    populateListWidget();
    qDebug() << "Loaded" << m_tasks.count() << "tasks.";
}

void TodoWidget::saveTasks()
{
    QSettings settings;
    settings.beginWriteArray(TASKS_ARRAY_KEY);
    for (int i = 0; i < m_tasks.size(); ++i) {
        settings.setArrayIndex(i);
        const TodoItem& task = m_tasks.at(i);
        settings.setValue("id", task.id);
        settings.setValue("description", task.description);
        settings.setValue("isCompleted", task.isCompleted);
        settings.setValue("createdAt", task.createdAt);
        settings.setValue("completedAt", task.completedAt);
    }
    settings.endArray();
    qDebug() << "Saved" << m_tasks.count() << "tasks.";
}

void TodoWidget::populateListWidget()
{
    m_taskListWidget->clear();
    for (const TodoItem& task : m_tasks) {
        QListWidgetItem* listItem = new QListWidgetItem(task.description, m_taskListWidget);
        listItem->setFlags(listItem->flags() | Qt::ItemIsUserCheckable);
        listItem->setCheckState(task.isCompleted ? Qt::Checked : Qt::Unchecked);
        listItem->setData(Qt::UserRole, task.id);
        QFont font = listItem->font();
        font.setStrikeOut(task.isCompleted);
        listItem->setFont(font);
        m_taskListWidget->addItem(listItem);
    }
}
