#include "scenewidget.h"
#include "tcpmanager.h"
#include <QMessageBox>

SceneWidget::SceneWidget(QWidget *parent) : QWidget(parent)
{
    setupUi();
    loadScenes();
}

void SceneWidget::setupUi()
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    m_sceneList = new QListWidget(this);
    m_sceneList->setObjectName("sceneList"); // QSS
    m_sceneList->setViewMode(QListWidget::IconMode);
    m_sceneList->setIconSize(QSize(100, 100));
    m_sceneList->setSpacing(20);
    m_sceneList->setResizeMode(QListWidget::Adjust);

    layout->addWidget(m_sceneList);

    connect(m_sceneList, &QListWidget::itemDoubleClicked, this, &SceneWidget::onActivateScene);
}

void SceneWidget::loadScenes()
{
    // Mock Scenes
    struct Scene
    {
        QString name;
        QString icon; // Path or placeholder
        QStringList commands;
    };

    QList<Scene> scenes;
    scenes.append({"🏠 回家模式", "", {"LIGHT_LIVING_ON", "AC_LIVING_ON"}});
    scenes.append({"🚪 离家模式", "", {"LIGHT_LIVING_OFF", "LIGHT_BEDROOM_OFF", "AC_LIVING_OFF"}});
    scenes.append({"🛌 睡眠模式", "", {"LIGHT_LIVING_OFF", "LIGHT_BEDROOM_OFF", "AC_TEMP_26"}});

    for (const auto &s : scenes)
    {
        QListWidgetItem *item = new QListWidgetItem(s.name);
        item->setSizeHint(QSize(120, 120));
        // Store commands in UserRole
        item->setData(Qt::UserRole, s.commands);
        m_sceneList->addItem(item);
    }
}

void SceneWidget::onActivateScene(QListWidgetItem *item)
{
    QString sceneName = item->text();
    QStringList commands = item->data(Qt::UserRole).toStringList();

    if (QMessageBox::question(this, "激活场景", "确认激活 " + sceneName + " ?") == QMessageBox::Yes)
    {
        for (const QString &cmd : commands)
        {
            TcpManager::instance().sendCommand(cmd);
        }
        QMessageBox::information(this, "成功", sceneName + " 已激活");
    }
}
