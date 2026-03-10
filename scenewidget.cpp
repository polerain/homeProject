#include "scenewidget.h"
#include "tcpmanager.h"
#include "sceneeditdialog.h"
#include <QMessageBox>
#include <QDebug>

SceneWidget::SceneWidget(QWidget *parent) : QWidget(parent)
{
    setupUi();
    loadScenes();

    // Set minimum size to prevent UI from being too small
    setMinimumSize(900, 600);

    // Connect to receive device status updates
    connect(&TcpManager::instance(), &TcpManager::dataReceived, this, [this](const QString &data)
            {
        // Reuse similar logic to DeviceControlWidget to update status
         if (data.contains(":"))
        {
            QStringList parts = data.split(",");
            for (const QString &part : parts)
            {
                QStringList kv = part.split(":");
                if (kv.size() == 2)
                {
                    QString key = kv[0].trimmed();
                    QString val = kv[1].trimmed();

                    for (int i = 0; i < m_deviceList->count(); ++i)
                    {
                        QListWidgetItem *item = m_deviceList->item(i);
                        DeviceItemWidget *widget = qobject_cast<DeviceItemWidget *>(m_deviceList->itemWidget(item));
                        if (widget && key.compare(widget->getId(), Qt::CaseInsensitive) == 0)
                        {
                            widget->setStatus(val);
                        }
                    }
                }
            }
        } });
}

void SceneWidget::setupUi()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);

    // Left: Scene List Area
    QVBoxLayout *leftLayout = new QVBoxLayout();

    // Header
    QLabel *sceneLabel = new QLabel("场景列表", this);
    sceneLabel->setStyleSheet("font-size: 16px; font-weight: bold; margin-bottom: 10px;");
    leftLayout->addWidget(sceneLabel);

    // Toolbar
    QHBoxLayout *toolLayout = new QHBoxLayout();
    m_addBtn = new QPushButton("新增", this);
    m_editBtn = new QPushButton("编辑", this);
    m_delBtn = new QPushButton("删除", this);
    m_activateBtn = new QPushButton("激活场景", this);

    toolLayout->addWidget(m_addBtn);
    toolLayout->addWidget(m_editBtn);
    toolLayout->addWidget(m_delBtn);

    leftLayout->addLayout(toolLayout);
    leftLayout->addWidget(m_activateBtn);

    m_sceneList = new QListWidget(this);
    m_sceneList->setObjectName("sceneList"); // QSS
    m_sceneList->setViewMode(QListWidget::IconMode);
    m_sceneList->setIconSize(QSize(80, 80));
    m_sceneList->setSpacing(15);
    m_sceneList->setResizeMode(QListWidget::Adjust);
    m_sceneList->setMaximumWidth(400); // Limit width of scene list

    leftLayout->addWidget(m_sceneList);

    // Right: Device List (Scene Details)
    QVBoxLayout *rightLayout = new QVBoxLayout();
    m_sceneDetailLabel = new QLabel("场景包含的设备", this);
    m_sceneDetailLabel->setStyleSheet("font-size: 16px; font-weight: bold; margin-bottom: 10px;");
    rightLayout->addWidget(m_sceneDetailLabel);

    m_deviceList = new QListWidget(this);
    m_deviceList->setObjectName("deviceList"); // Reuse QSS from DeviceControlWidget
    m_deviceList->setSelectionMode(QAbstractItemView::SingleSelection);

    rightLayout->addWidget(m_deviceList);

    mainLayout->addLayout(leftLayout, 1);
    mainLayout->addLayout(rightLayout, 2);

    connect(m_sceneList, &QListWidget::itemDoubleClicked, this, &SceneWidget::onActivateScene);
    connect(m_sceneList, &QListWidget::itemClicked, this, &SceneWidget::onSceneSelected);

    connect(m_addBtn, &QPushButton::clicked, this, &SceneWidget::onAddScene);
    connect(m_editBtn, &QPushButton::clicked, this, &SceneWidget::onEditScene);
    connect(m_delBtn, &QPushButton::clicked, this, &SceneWidget::onDeleteScene);
    connect(m_activateBtn, &QPushButton::clicked, this, &SceneWidget::onActivateBtnClicked);

    // Initial state
    m_editBtn->setEnabled(false);
    m_delBtn->setEnabled(false);
    m_activateBtn->setEnabled(false);
}

void SceneWidget::loadScenes()
{
    m_sceneList->clear();
    QList<SceneInfo> scenes = DatabaseManager::instance().getAllScenes();

    for (const auto &s : scenes)
    {
        if (!s.isEnabled)
            continue; // Skip disabled scenes if needed, or show them grayed out

        QListWidgetItem *item = new QListWidgetItem(s.name);
        item->setSizeHint(QSize(100, 100));

        // Store ID in UserRole
        item->setData(Qt::UserRole, s.id);
        item->setData(Qt::UserRole + 1, s.isBuiltin);

        if (s.isBuiltin)
        {
            item->setToolTip("内置场景");
            item->setText(s.name);
        }
        else
        {
            item->setText(s.name);
        }

        m_sceneList->addItem(item);
    }
}

void SceneWidget::onActivateScene(QListWidgetItem *item)
{
    if (!item)
        return;
    int sceneId = item->data(Qt::UserRole).toInt();
    QString sceneName = item->text().split("\n").first(); // Remove suffix

    if (QMessageBox::question(this, "激活场景", "确认激活 " + sceneName + " ?") == QMessageBox::Yes)
    {
        executeScene(sceneId);
    }
}

void SceneWidget::onActivateBtnClicked()
{
    QListWidgetItem *item = m_sceneList->currentItem();
    if (item)
        onActivateScene(item);
}

void SceneWidget::onSceneSelected(QListWidgetItem *item)
{
    if (!item)
        return;

    int sceneId = item->data(Qt::UserRole).toInt();
    bool isBuiltin = item->data(Qt::UserRole + 1).toBool();
    QString sceneName = item->text().split("\n").first();

    m_sceneDetailLabel->setText(sceneName + " - 设备控制预览");
    loadSceneDevices(sceneId);

    // Update button states
    m_editBtn->setEnabled(!isBuiltin);
    m_delBtn->setEnabled(!isBuiltin);
    m_activateBtn->setEnabled(true);
}

void SceneWidget::loadSceneDevices(int sceneId)
{
    m_deviceList->clear();

    QList<SceneDeviceBinding> bindings = DatabaseManager::instance().getSceneBindings(sceneId);

    for (const auto &b : bindings)
    {
        if (!b.isSelected)
            continue;

        QListWidgetItem *item = new QListWidgetItem(m_deviceList);

        // Find current device info to get type for UI size hint
        DeviceInfo devInfo = DeviceManager::instance().getDeviceById(b.deviceId);
        if (devInfo.id.isEmpty())
        {
            // Fallback if device not found in manager (maybe deleted?)
            devInfo.name = b.deviceName;
            devInfo.type = b.deviceType;
            devInfo.id = b.deviceId;
        }

        if (devInfo.type == "AC" || devInfo.type == "LIGHT")
        {
            item->setSizeHint(QSize(0, 120));
        }
        else
        {
            item->setSizeHint(QSize(0, 90));
        }

        DeviceItemWidget *widget = new DeviceItemWidget(devInfo.name, devInfo.type, devInfo.status, devInfo.id);
        connect(widget, &DeviceItemWidget::controlClicked, this, &SceneWidget::onDeviceControl);

        m_deviceList->setItemWidget(item, widget);
    }

    // Trigger a status refresh to get current states
    TcpManager::instance().sendCommand("GET_ALL_STATUS");
}

void SceneWidget::onDeviceControl(const QString &cmd)
{
    TcpManager::instance().sendCommand(cmd);
}

void SceneWidget::onAddScene()
{
    SceneEditDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted)
    {
        SceneInfo info = dlg.getSceneInfo();
        if (DatabaseManager::instance().addScene(info))
        {
            DatabaseManager::instance().saveSceneBindings(info.id, dlg.getBindings());
            loadScenes();
            QMessageBox::information(this, "成功", "场景已添加");
        }
        else
        {
            QMessageBox::critical(this, "错误", "添加场景失败");
        }
    }
}

void SceneWidget::onEditScene()
{
    QListWidgetItem *item = m_sceneList->currentItem();
    if (!item)
        return;

    int sceneId = item->data(Qt::UserRole).toInt();
    bool isBuiltin = item->data(Qt::UserRole + 1).toBool();

    if (isBuiltin)
    {
        QMessageBox::information(this, "提示", "内置场景不可编辑");
        return;
    }

    SceneEditDialog dlg(this);
    SceneInfo info = DatabaseManager::instance().getSceneById(sceneId);
    dlg.setSceneInfo(info);
    dlg.setBindings(DatabaseManager::instance().getSceneBindings(sceneId));

    if (dlg.exec() == QDialog::Accepted)
    {
        SceneInfo newInfo = dlg.getSceneInfo();
        newInfo.id = sceneId; // Ensure ID is kept

        if (DatabaseManager::instance().updateScene(newInfo))
        {
            DatabaseManager::instance().saveSceneBindings(sceneId, dlg.getBindings());
            loadScenes();
            // Reselect
            for (int i = 0; i < m_sceneList->count(); ++i)
            {
                if (m_sceneList->item(i)->data(Qt::UserRole).toInt() == sceneId)
                {
                    m_sceneList->setCurrentRow(i);
                    onSceneSelected(m_sceneList->item(i));
                    break;
                }
            }
            QMessageBox::information(this, "成功", "场景已更新");
        }
        else
        {
            QMessageBox::critical(this, "错误", "更新场景失败");
        }
    }
}

void SceneWidget::onDeleteScene()
{
    QListWidgetItem *item = m_sceneList->currentItem();
    if (!item)
        return;

    int sceneId = item->data(Qt::UserRole).toInt();
    bool isBuiltin = item->data(Qt::UserRole + 1).toBool();
    QString name = item->text().split("\n").first();

    if (isBuiltin)
    {
        QMessageBox::warning(this, "警告", "内置场景不可删除");
        return;
    }

    if (QMessageBox::question(this, "确认删除", "确定要删除场景 " + name + " 吗？") == QMessageBox::Yes)
    {
        if (DatabaseManager::instance().deleteScene(sceneId))
        {
            loadScenes();
            m_deviceList->clear();
            m_sceneDetailLabel->setText("场景包含的设备");
            m_editBtn->setEnabled(false);
            m_delBtn->setEnabled(false);
            m_activateBtn->setEnabled(false);
            QMessageBox::information(this, "成功", "场景已删除");
        }
        else
        {
            QMessageBox::critical(this, "错误", "删除场景失败");
        }
    }
}

void SceneWidget::executeScene(int sceneId)
{
    if (!TcpManager::instance().isConnected())
    {
        QMessageBox::warning(this, "错误", "未连接到设备网关，无法执行场景");
        return;
    }

    QList<SceneDeviceBinding> bindings = DatabaseManager::instance().getSceneBindings(sceneId);
    if (bindings.isEmpty())
    {
        QMessageBox::warning(this, "提示", "该场景未绑定任何设备");
        return;
    }

    int cmdCount = 0;

    for (const auto &b : bindings)
    {
        if (!b.isSelected)
            continue;

        QString id = b.deviceId.toUpper();
        QStringList cmds;

        // Power (Priority 1)
        if (!b.targetPower.isEmpty())
        {
            cmds << QString("%1_%2").arg(id).arg(b.targetPower);
        }

        // Attributes
        if (b.deviceType == "LIGHT")
        {
            if (b.targetBrightness >= 0)
            {
                cmds << QString("%1_BRI_%2").arg(id).arg(b.targetBrightness);
            }
            if (!b.targetLightColor.isEmpty())
            {
                cmds << QString("%1_COLOR_%2").arg(id).arg(b.targetLightColor);
            }
        }
        else if (b.deviceType == "AC")
        {
            if (b.targetTemp > 0)
            {
                cmds << QString("%1_TEMP_%2").arg(id).arg(b.targetTemp);
            }
            if (!b.targetMode.isEmpty())
            {
                cmds << QString("%1_MODE_%2").arg(id).arg(b.targetMode);
            }
            if (!b.targetFan.isEmpty())
            {
                cmds << QString("%1_FAN_%2").arg(id).arg(b.targetFan);
            }
        }
        else if (b.deviceType == "CURTAIN")
        {
            if (!b.targetPosition.isEmpty())
            {
                if (b.targetPosition == "TOGGLE")
                    cmds << QString("%1_TOGGLE").arg(id);
                else
                    cmds << QString("%1_%2").arg(id).arg(b.targetPosition);
            }
        }

        for (const QString &cmd : cmds)
        {
            TcpManager::instance().sendCommand(cmd);
            cmdCount++;
        }
    }

    QMessageBox::information(this, "执行完成", QString("已发送 %1 条指令").arg(cmdCount));
    TcpManager::instance().sendCommand("GET_ALL_STATUS");
}
