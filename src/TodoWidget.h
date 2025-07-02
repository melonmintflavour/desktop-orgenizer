#ifndef TODOWIDGET_H
#define TODOWIDGET_H

#include <QWidget>
#include <QList>
#include "TodoData.h" // Include the data structure

class QLineEdit;
class QPushButton;
class QListWidget;
class QListWidgetItem;
class QVBoxLayout;

class TodoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TodoWidget(QWidget *parent = nullptr);
    ~TodoWidget() override;

protected:
    void closeEvent(QCloseEvent *event) override; // To save tasks when widget host is closed

private slots:
    void handleAddTask();
    void handleRemoveTask();
    void handleClearCompletedTasks();
    void handleTaskItemChanged(QListWidgetItem *item);

private:
    void setupUI();
    void loadTasks();
    void saveTasks();
    void populateListWidget();
    TodoItem* findTaskById(const QUuid& id);
    QListWidgetItem* findListWidgetItemById(const QUuid& id);

    QLineEdit* m_taskInputLineEdit;
    QPushButton* m_addTaskButton;
    QListWidget* m_taskListWidget;
    QPushButton* m_removeTaskButton;
    QPushButton* m_clearCompletedButton;

    QList<TodoItem> m_tasks;
};

#endif // TODOWIDGET_H
