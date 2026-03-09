#include "scenewidget.h"
#include "tcpmanager.h"
#include "databasemanager.h"
#include "sceneeditdialog.h"
#include "sceneexecutor.h"
#include "sceneitemwidget.h"
#include <QMessageBox>
#include <QHBoxLayout>
#include <QDebug>

SceneWidget::SceneWidget(QWidget *parent) : QWidget(parent)
{
    setupUi();
    loadScenes();
}

void SceneWidget::setupUi()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    // Top Toolbar
    QHBoxLayout *toolbar = new QHBoxLayout();
    m_btnAdd = new QPushButton("新增场景");
    m_btnEdit = new QPushButton("编辑场景");
    m_btnDelete = new QPushButton("删除场景");
    m_btnActivate = new QPushButton("激活场景");
    m_btnRefresh = new QPushButton("刷新列表");
    
    // Style
    m_btnAdd->setStyleSheet("background-color: #28a745; color: white; padding: 5px 10px; border-radius: 4px;");
    m_btnActivate->setStyleSheet("background-color: #007bff; color: white; padding: 5px 10px; border-radius: 4px;");
    m_btnDelete->setStyleSheet("background-color: #dc3545; color: white; padding: 5px 10px; border-radius: 4px;");
    
    toolbar->addWidget(m_btnAdd);
    toolbar->addWidget(m_btnEdit);
    toolbar->addWidget(m_btnDelete);
    toolbar->addStretch();
    toolbar->addWidget(m_btnActivate);
    toolbar->addWidget(m_btnRefresh);
    
    layout->addLayout(toolbar);
    
    // List
    m_sceneList = new QListWidget(this);
    m_sceneList->setSpacing(10);
    // Use ListMode for vertical list
    m_sceneList->setViewMode(QListWidget::ListMode);
    m_sceneList->setStyleSheet("QListWidget::item { border-bottom: 1px solid #eee; padding: 5px; }");
    
    layout->addWidget(m_sceneList);
    
    // Connects
    connect(m_btnAdd, &QPushButton::clicked, this, &SceneWidget::onAddClicked);
    connect(m_btnEdit, &QPushButton::clicked, this, &SceneWidget::onEditClicked);
    connect(m_btnDelete, &QPushButton::clicked, this, &SceneWidget::onDeleteClicked);
    connect(m_btnActivate, &QPushButton::clicked, this, &SceneWidget::onActivateClicked);
    connect(m_btnRefresh, &QPushButton::clicked, this, &SceneWidget::onRefreshClicked);
    connect(m_sceneList, &QListWidget::itemDoubleClicked, this, &SceneWidget::onSceneDoubleClicked);
}

void SceneWidget::loadScenes()
{
    m_sceneList->clear();
    auto scenes = DatabaseManager::instance().getAllScenes();
    
    for(const auto &s : scenes) {
        // Fetch bindings for summary
        auto bindings = DatabaseManager::instance().getSceneBindings(s.id);
        QStringList summaryList;
        int maxItems = 6; // Increased from 3
        for (int i = 0; i < bindings.size() && i < maxItems; ++i) {
            const auto &b = bindings[i];
            QString desc = b.deviceName + ": " + b.targetState;
            if (!b.targetValue.isEmpty()) desc += " (" + b.targetValue + ")";
            summaryList.append(desc);
        }
        if (bindings.size() > maxItems) {
            summaryList.append(QString("... 等共%1个设备").arg(bindings.size()));
        }
        QString summary = summaryList.isEmpty() ? "无设备绑定" : summaryList.join(" | ");

        // Create Item
        QListWidgetItem *item = new QListWidgetItem(m_sceneList);
        item->setSizeHint(QSize(0, 100)); // Increased height from 80
        item->setData(Qt::UserRole, s.id);
        
        // Create Custom Widget
        SceneItemWidget *widget = new SceneItemWidget(s, summary);
        m_sceneList->setItemWidget(item, widget);
    }
}

void SceneWidget::onAddClicked()
{
    SceneEditDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        auto sceneData = dialog.getSceneData();
        auto bindings = dialog.getBindings();
        
        if (DatabaseManager::instance().addScene(sceneData, bindings)) {
            QMessageBox::information(this, "成功", "场景添加成功");
            loadScenes();
        } else {
            QMessageBox::critical(this, "失败", "场景添加失败，请查看日志");
        }
    }
}

void SceneWidget::onEditClicked()
{
    QListWidgetItem *item = m_sceneList->currentItem();
    if (!item) {
        QMessageBox::warning(this, "提示", "请先选择一个场景");
        return;
    }
    
    // Double click logic is reused for Edit
    onSceneDoubleClicked(item);
}

void SceneWidget::onDeleteClicked()
{
    QListWidgetItem *item = m_sceneList->currentItem();
    if (!item) {
        QMessageBox::warning(this, "提示", "请先选择一个场景");
        return;
    }
    
    int sceneId = item->data(Qt::UserRole).toInt();
    if (QMessageBox::question(this, "确认", "确定要删除该场景吗？此操作不可恢复。") == QMessageBox::Yes) {
        if (DatabaseManager::instance().deleteScene(sceneId)) {
            QMessageBox::information(this, "成功", "场景已删除");
            loadScenes();
        } else {
            QMessageBox::critical(this, "失败", "删除失败，请查看日志");
        }
    }
}

void SceneWidget::onActivateClicked()
{
    QListWidgetItem *item = m_sceneList->currentItem();
    if (!item) {
        QMessageBox::warning(this, "提示", "请先选择一个场景");
        return;
    }
    
    int sceneId = item->data(Qt::UserRole).toInt();
    
    SceneExecutor executor;
    auto result = executor.execute(sceneId);
    
    if (result.success) {
        QString msg = QString("场景执行成功\n总指令: %1\n成功: %2")
                      .arg(result.totalCommands)
                      .arg(result.successCount);
        if (result.failCount > 0) {
             msg += QString("\n失败: %1\n失败设备: %2")
                    .arg(result.failCount)
                    .arg(result.failedDevices.join(", "));
             QMessageBox::warning(this, "部分执行成功", msg);
        } else {
             QMessageBox::information(this, "执行成功", msg);
        }
        loadScenes(); 
    } else {
        QString msg = "场景执行失败:\n";
        msg += result.errorMessages.join("\n");
        QMessageBox::critical(this, "执行失败", msg);
    }
}

void SceneWidget::onRefreshClicked()
{
    loadScenes();
}

void SceneWidget::onSceneDoubleClicked(QListWidgetItem *item)
{
    // Changed behavior: Double click -> Edit
    int sceneId = item->data(Qt::UserRole).toInt();
    SceneEditDialog dialog(this, sceneId);
    if (dialog.exec() == QDialog::Accepted) {
        auto sceneData = dialog.getSceneData();
        auto bindings = dialog.getBindings();
        
        if (DatabaseManager::instance().updateScene(sceneData, bindings)) {
            QMessageBox::information(this, "成功", "场景更新成功");
            loadScenes();
        } else {
            QMessageBox::critical(this, "失败", "场景更新失败，请查看日志");
        }
    }
}
