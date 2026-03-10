#include "sceneeditdialog.h"
#include <QMessageBox>
#include <QLabel>
#include <QGroupBox>

// --- SceneDeviceItemWidget Implementation ---

SceneDeviceItemWidget::SceneDeviceItemWidget(const DeviceInfo &device, QWidget *parent)
    : QWidget(parent), m_device(device)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);

    m_selectCb = new QCheckBox(this);
    m_nameLabel = new QLabel(QString("%1 (%2)").arg(device.name).arg(device.type), this);
    m_nameLabel->setMinimumWidth(120);

    layout->addWidget(m_selectCb);
    layout->addWidget(m_nameLabel);

    // Control Area
    m_controlArea = new QWidget(this);
    QHBoxLayout *controlLayout = new QHBoxLayout(m_controlArea);
    controlLayout->setContentsMargins(0, 0, 0, 0);

    // Common Power Control (for Light and AC)
    if (device.type == "LIGHT" || device.type == "AC" || device.type == "FAN")
    {
        m_powerCombo = new QComboBox(this);
        m_powerCombo->addItems({"开启", "关闭"});
        m_powerCombo->setCurrentText("开启");
        controlLayout->addWidget(new QLabel("电源:", this));
        controlLayout->addWidget(m_powerCombo);
    }

    if (device.type == "LIGHT")
    {
        controlLayout->addWidget(new QLabel("亮度:", this));
        m_brightnessSpin = new QSpinBox(this);
        m_brightnessSpin->setRange(0, 100);
        m_brightnessSpin->setValue(80);
        m_brightnessSpin->setSuffix("%");
        controlLayout->addWidget(m_brightnessSpin);

        controlLayout->addWidget(new QLabel("色温:", this));
        m_lightModeCombo = new QComboBox(this);
        m_lightModeCombo->addItems({"暖光", "自然光", "冷白光"});
        m_lightModeCombo->setCurrentText("暖光");
        controlLayout->addWidget(m_lightModeCombo);
    }
    else if (device.type == "AC")
    {
        controlLayout->addWidget(new QLabel("温度:", this));
        m_tempSpin = new QSpinBox(this);
        m_tempSpin->setRange(16, 30);
        m_tempSpin->setValue(26);
        m_tempSpin->setSuffix("°C");
        controlLayout->addWidget(m_tempSpin);

        controlLayout->addWidget(new QLabel("模式:", this));
        m_acModeCombo = new QComboBox(this);
        m_acModeCombo->addItems({"制冷", "制暖", "送风", "除湿"});
        m_acModeCombo->setCurrentText("制冷");
        controlLayout->addWidget(m_acModeCombo);

        controlLayout->addWidget(new QLabel("风速:", this));
        m_fanCombo = new QComboBox(this);
        m_fanCombo->addItems({"自动", "低风", "中风", "高风"});
        m_fanCombo->setCurrentText("自动");
        controlLayout->addWidget(m_fanCombo);
    }
    else if (device.type == "CURTAIN")
    {
        controlLayout->addWidget(new QLabel("动作:", this));
        m_curtainActionCombo = new QComboBox(this);
        m_curtainActionCombo->addItems({"保持不变", "打开", "关闭", "开/关"});
        controlLayout->addWidget(m_curtainActionCombo);
    }

    controlLayout->addStretch();
    layout->addWidget(m_controlArea);

    // Disable controls if not selected
    connect(m_selectCb, &QCheckBox::toggled, m_controlArea, &QWidget::setEnabled);
    m_controlArea->setEnabled(false); // Default disabled
}

void SceneDeviceItemWidget::setBinding(const SceneDeviceBinding &binding)
{
    m_selectCb->setChecked(binding.isSelected);

    if (m_device.type == "LIGHT" || m_device.type == "AC" || m_device.type == "FAN")
    {
        if (binding.targetPower == "ON")
            m_powerCombo->setCurrentText("开启");
        else if (binding.targetPower == "OFF")
            m_powerCombo->setCurrentText("关闭");
        else
            m_powerCombo->setCurrentText("开启");
    }

    if (m_device.type == "LIGHT")
    {
        if (binding.targetBrightness >= 0)
            m_brightnessSpin->setValue(binding.targetBrightness);
        else
            m_brightnessSpin->setValue(80);

        if (binding.targetLightColor == "WARM")
            m_lightModeCombo->setCurrentText("暖光");
        else if (binding.targetLightColor == "NATURAL")
            m_lightModeCombo->setCurrentText("自然光");
        else if (binding.targetLightColor == "COLD")
            m_lightModeCombo->setCurrentText("冷白光");
        else
            m_lightModeCombo->setCurrentText("暖光");
    }
    else if (m_device.type == "AC")
    {
        if (binding.targetTemp > 0)
            m_tempSpin->setValue(binding.targetTemp);
        else
            m_tempSpin->setValue(26);

        if (binding.targetMode == "COOL")
            m_acModeCombo->setCurrentText("制冷");
        else if (binding.targetMode == "HEAT")
            m_acModeCombo->setCurrentText("制暖");
        else if (binding.targetMode == "FAN")
            m_acModeCombo->setCurrentText("送风");
        else if (binding.targetMode == "DRY")
            m_acModeCombo->setCurrentText("除湿");
        else
            m_acModeCombo->setCurrentText("制冷");

        if (binding.targetFan == "AUTO")
            m_fanCombo->setCurrentText("自动");
        else if (binding.targetFan == "LOW")
            m_fanCombo->setCurrentText("低风");
        else if (binding.targetFan == "MID")
            m_fanCombo->setCurrentText("中风");
        else if (binding.targetFan == "HIGH")
            m_fanCombo->setCurrentText("高风");
        else
            m_fanCombo->setCurrentText("自动");
    }
    else if (m_device.type == "CURTAIN")
    {
        if (binding.targetPosition == "OPEN")
            m_curtainActionCombo->setCurrentText("打开");
        else if (binding.targetPosition == "CLOSE")
            m_curtainActionCombo->setCurrentText("关闭");
        else if (binding.targetPosition == "TOGGLE")
            m_curtainActionCombo->setCurrentText("开/关");
        else
            m_curtainActionCombo->setCurrentText("保持不变");
    }
}

SceneDeviceBinding SceneDeviceItemWidget::getBinding() const
{
    SceneDeviceBinding b;
    b.deviceId = m_device.id;
    b.deviceName = m_device.name;
    b.deviceType = m_device.type;
    b.isSelected = m_selectCb->isChecked();

    // Defaults
    b.targetPower = "";
    b.targetTemp = -1;
    b.targetBrightness = -1;
    b.targetPosition = "";
    b.targetMode = "";
    b.targetFan = "";
    b.targetLightColor = "";

    if (!b.isSelected)
        return b;

    if (m_device.type == "LIGHT" || m_device.type == "AC" || m_device.type == "FAN")
    {
        QString txt = m_powerCombo->currentText();
        if (txt == "开启")
            b.targetPower = "ON";
        else if (txt == "关闭")
            b.targetPower = "OFF";
    }

    if (m_device.type == "LIGHT")
    {
        b.targetBrightness = m_brightnessSpin->value();

        QString color = m_lightModeCombo->currentText();
        if (color == "暖光")
            b.targetLightColor = "WARM";
        else if (color == "自然光")
            b.targetLightColor = "NATURAL";
        else if (color == "冷白光")
            b.targetLightColor = "COLD";
    }
    else if (m_device.type == "AC")
    {
        b.targetTemp = m_tempSpin->value();

        QString mode = m_acModeCombo->currentText();
        if (mode == "制冷")
            b.targetMode = "COOL";
        else if (mode == "制暖")
            b.targetMode = "HEAT";
        else if (mode == "送风")
            b.targetMode = "FAN";
        else if (mode == "除湿")
            b.targetMode = "DRY";

        QString fan = m_fanCombo->currentText();
        if (fan == "自动")
            b.targetFan = "AUTO";
        else if (fan == "低风")
            b.targetFan = "LOW";
        else if (fan == "中风")
            b.targetFan = "MID";
        else if (fan == "高风")
            b.targetFan = "HIGH";
    }
    else if (m_device.type == "CURTAIN")
    {
        QString action = m_curtainActionCombo->currentText();
        if (action == "打开")
            b.targetPosition = "OPEN";
        else if (action == "关闭")
            b.targetPosition = "CLOSE";
        else if (action == "开/关")
            b.targetPosition = "TOGGLE";
    }

    return b;
}

bool SceneDeviceItemWidget::isSelected() const
{
    return m_selectCb->isChecked();
}

// --- SceneEditDialog Implementation ---

SceneEditDialog::SceneEditDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("场景编辑");
    resize(600, 500);
    setMinimumSize(600, 500);
    setupUi();
    loadDevices();
}

void SceneEditDialog::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 1. Basic Info
    QGroupBox *infoGroup = new QGroupBox("基本信息", this);
    QGridLayout *infoLayout = new QGridLayout(infoGroup);

    infoLayout->addWidget(new QLabel("场景名称:", this), 0, 0);
    m_nameEdit = new QLineEdit(this);
    infoLayout->addWidget(m_nameEdit, 0, 1);

    infoLayout->addWidget(new QLabel("场景描述:", this), 1, 0);
    m_descEdit = new QLineEdit(this);
    infoLayout->addWidget(m_descEdit, 1, 1);

    m_enabledCb = new QCheckBox("启用场景", this);
    m_enabledCb->setChecked(true);
    infoLayout->addWidget(m_enabledCb, 2, 1);

    mainLayout->addWidget(infoGroup);

    // 2. Devices
    QGroupBox *devGroup = new QGroupBox("设备控制 (勾选以加入场景)", this);
    QVBoxLayout *devLayout = new QVBoxLayout(devGroup);

    m_deviceList = new QListWidget(this);
    m_deviceList->setSelectionMode(QAbstractItemView::NoSelection); // Items have their own controls
    devLayout->addWidget(m_deviceList);

    mainLayout->addWidget(devGroup);

    // 3. Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    QPushButton *cancelBtn = new QPushButton("取消", this);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    QPushButton *saveBtn = new QPushButton("保存", this);
    connect(saveBtn, &QPushButton::clicked, this, &SceneEditDialog::onSave);

    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(saveBtn);

    mainLayout->addLayout(btnLayout);
}

void SceneEditDialog::loadDevices()
{
    m_deviceList->clear();
    QList<DeviceInfo> devices = DeviceManager::instance().getAllDevices();

    for (const auto &dev : devices)
    {
        QListWidgetItem *item = new QListWidgetItem(m_deviceList);
        item->setSizeHint(QSize(0, 60)); // Set height

        SceneDeviceItemWidget *widget = new SceneDeviceItemWidget(dev);
        m_deviceList->setItemWidget(item, widget);
    }
}

void SceneEditDialog::setSceneInfo(const SceneInfo &info)
{
    m_sceneInfo = info;
    m_nameEdit->setText(info.name);
    m_descEdit->setText(info.description);
    m_enabledCb->setChecked(info.isEnabled);
}

SceneInfo SceneEditDialog::getSceneInfo() const
{
    SceneInfo info = m_sceneInfo;
    info.name = m_nameEdit->text().trimmed();
    info.description = m_descEdit->text().trimmed();
    info.isEnabled = m_enabledCb->isChecked();
    return info;
}

void SceneEditDialog::setBindings(const QList<SceneDeviceBinding> &bindings)
{
    // Iterate over all list items and match bindings
    for (int i = 0; i < m_deviceList->count(); ++i)
    {
        QListWidgetItem *item = m_deviceList->item(i);
        SceneDeviceItemWidget *widget = qobject_cast<SceneDeviceItemWidget *>(m_deviceList->itemWidget(item));
        if (widget)
        {
            SceneDeviceBinding binding = widget->getBinding(); // Get current (default)
            // Find if there is a binding for this device
            for (const auto &b : bindings)
            {
                if (b.deviceId == binding.deviceId)
                {
                    widget->setBinding(b);
                    break;
                }
            }
        }
    }
}

QList<SceneDeviceBinding> SceneEditDialog::getBindings() const
{
    QList<SceneDeviceBinding> list;
    for (int i = 0; i < m_deviceList->count(); ++i)
    {
        QListWidgetItem *item = m_deviceList->item(i);
        SceneDeviceItemWidget *widget = qobject_cast<SceneDeviceItemWidget *>(m_deviceList->itemWidget(item));
        if (widget && widget->isSelected())
        {
            list.append(widget->getBinding());
        }
    }
    return list;
}

void SceneEditDialog::onSave()
{
    if (m_nameEdit->text().trimmed().isEmpty())
    {
        QMessageBox::warning(this, "警告", "场景名称不能为空");
        return;
    }

    if (getBindings().isEmpty())
    {
        QMessageBox::warning(this, "警告", "请至少选择一个设备加入场景");
        return;
    }

    accept();
}
