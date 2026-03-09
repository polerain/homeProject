#include "sceneeditdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QDebug>
#include <QGroupBox>
#include <QFormLayout>

SceneEditDialog::SceneEditDialog(QWidget *parent, int sceneId)
    : QDialog(parent), m_sceneId(sceneId)
{
    setupUi();
    populateDeviceList();
    if (m_sceneId != -1)
    {
        loadData();
        setWindowTitle("编辑场景");
    }
    else
    {
        setWindowTitle("新增场景");
    }
    resize(600, 700);
}

void SceneEditDialog::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 1. Scene Info
    QGroupBox *infoGroup = new QGroupBox("基本信息", this);
    QFormLayout *formLayout = new QFormLayout(infoGroup);

    m_nameEdit = new QLineEdit();
    m_descEdit = new QTextEdit();
    m_descEdit->setMaximumHeight(60);
    m_enabledCheck = new QCheckBox("启用该场景");
    m_enabledCheck->setChecked(true);

    formLayout->addRow("场景名称:", m_nameEdit);
    formLayout->addRow("场景描述:", m_descEdit);
    formLayout->addRow("", m_enabledCheck);

    mainLayout->addWidget(infoGroup);

    // 2. Device Bindings
    QGroupBox *deviceGroup = new QGroupBox("设备绑定", this);
    QVBoxLayout *deviceLayout = new QVBoxLayout(deviceGroup);

    m_deviceList = new QListWidget();
    m_deviceList->setSelectionMode(QAbstractItemView::NoSelection);
    deviceLayout->addWidget(m_deviceList);

    mainLayout->addWidget(deviceGroup);

    // 3. Buttons
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, [this]()
            {
        if(validate()) accept(); });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttons);
}

void SceneEditDialog::populateDeviceList()
{
    auto devices = DatabaseManager::instance().getAllDevices();
    m_rows.clear();

    for (const auto &dev : devices)
    {
        QListWidgetItem *item = new QListWidgetItem(m_deviceList);
        QWidget *widget = new QWidget();
        QHBoxLayout *layout = new QHBoxLayout(widget);
        layout->setContentsMargins(5, 5, 5, 5);

        // Checkbox + Name
        QCheckBox *chk = new QCheckBox(dev.name);
        layout->addWidget(chk);

        // Type Label
        QLabel *typeLabel = new QLabel(dev.type);
        typeLabel->setStyleSheet("color: gray; font-size: 10px; border: 1px solid #ccc; border-radius: 3px; padding: 2px;");
        layout->addWidget(typeLabel);

        layout->addStretch();

        // Controls
        QComboBox *stateCombo = new QComboBox();
        QSpinBox *valSpin = new QSpinBox();
        QComboBox *modeCombo = new QComboBox();
        QComboBox *fanCombo = new QComboBox();

        // Defaults
        valSpin->setVisible(false);
        modeCombo->setVisible(false);
        fanCombo->setVisible(false);

        if (dev.type == "LIGHT")
        {
            stateCombo->addItems({"ON", "OFF"});
        }
        else if (dev.type == "CURTAIN")
        {
            stateCombo->addItems({"OPEN", "CLOSE", "STOP"});
        }
        else if (dev.type == "AC")
        {
            stateCombo->addItems({"ON", "OFF", "TEMP", "MODE", "FAN"});

            // AC Extras
            valSpin->setRange(16, 30);
            valSpin->setValue(26);
            valSpin->setSuffix("°C");

            modeCombo->addItems({"COOL", "HEAT", "FAN", "DRY"});
            fanCombo->addItems({"AUTO", "LOW", "MID", "HIGH"});

            // Logic to show/hide based on state
            connect(stateCombo, &QComboBox::currentTextChanged, [=](const QString &text)
                    {
                valSpin->setVisible(text == "TEMP");
                modeCombo->setVisible(text == "MODE");
                fanCombo->setVisible(text == "FAN"); });
        }

        layout->addWidget(stateCombo);
        layout->addWidget(valSpin);
        layout->addWidget(modeCombo);
        layout->addWidget(fanCombo);

        item->setSizeHint(QSize(widget->sizeHint().width(), 50));
        m_deviceList->setItemWidget(item, widget);

        DeviceRow row;
        row.deviceId = dev.id;
        row.deviceName = dev.name;
        row.identifier = dev.identifier;
        row.type = dev.type;
        row.bindCheck = chk;
        row.stateCombo = stateCombo;
        row.valueSpin = valSpin;
        row.modeCombo = modeCombo;
        row.fanCombo = fanCombo;

        m_rows.append(row);
    }
}

void SceneEditDialog::loadData()
{
    auto scene = DatabaseManager::instance().getSceneById(m_sceneId);
    m_nameEdit->setText(scene.name);
    m_descEdit->setText(scene.description);
    m_enabledCheck->setChecked(scene.isEnabled);

    auto bindings = DatabaseManager::instance().getSceneBindings(m_sceneId);
    for (const auto &bind : bindings)
    {
        for (const auto &row : m_rows)
        {
            if (row.deviceId == bind.deviceId)
            {
                row.bindCheck->setChecked(true);

                // Restore state
                // If AC and state is specific (TEMP/MODE/FAN), we need to set combo and value
                // bind.targetState might be "TEMP", bind.targetValue "26"
                // OR bind.targetState "ON"

                // Find index
                int idx = row.stateCombo->findText(bind.targetState);
                if (idx >= 0)
                    row.stateCombo->setCurrentIndex(idx);

                // Restore value
                if (bind.targetState == "TEMP")
                {
                    row.valueSpin->setValue(bind.targetValue.toInt());
                }
                else if (bind.targetState == "MODE")
                {
                    row.modeCombo->setCurrentText(bind.targetValue);
                }
                else if (bind.targetState == "FAN")
                {
                    row.fanCombo->setCurrentText(bind.targetValue);
                }

                break;
            }
        }
    }
}

bool SceneEditDialog::validate()
{
    QString name = m_nameEdit->text().trimmed();
    if (name.isEmpty())
    {
        QMessageBox::warning(this, "校验失败", "场景名称不能为空");
        return false;
    }

    if (DatabaseManager::instance().checkSceneNameExists(name, m_sceneId))
    {
        QMessageBox::warning(this, "校验失败", "场景名称已存在，请使用其他名称");
        return false;
    }

    // Check if at least one device is selected? Not strictly required by prompt but good practice.
    // Prompt 5 says: "Scene has no bindings" -> Handle exception.
    // So we can allow saving empty scene but warn, or just allow it.
    // Let's count
    int count = 0;
    for (const auto &row : m_rows)
    {
        if (row.bindCheck->isChecked())
            count++;
    }
    if (count == 0)
    {
        QMessageBox::warning(this, "提示", "您未绑定任何设备，该场景执行时将无效果");
        // Proceed anyway? Yes.
    }

    return true;
}

DatabaseManager::SceneData SceneEditDialog::getSceneData() const
{
    DatabaseManager::SceneData d;
    d.id = m_sceneId;
    d.name = m_nameEdit->text().trimmed();
    d.description = m_descEdit->toPlainText();
    d.isEnabled = m_enabledCheck->isChecked();
    // Icon default
    d.icon = "";
    return d;
}

QList<DatabaseManager::SceneDeviceBinding> SceneEditDialog::getBindings() const
{
    QList<DatabaseManager::SceneDeviceBinding> list;
    for (const auto &row : m_rows)
    {
        if (row.bindCheck->isChecked())
        {
            DatabaseManager::SceneDeviceBinding b;
            b.deviceId = row.deviceId;
            b.deviceName = row.deviceName; // Optional, mainly for display
            b.targetState = row.stateCombo->currentText();

            // Determine value based on state
            if (b.targetState == "TEMP")
            {
                b.targetValue = QString::number(row.valueSpin->value());
            }
            else if (b.targetState == "MODE")
            {
                b.targetValue = row.modeCombo->currentText();
            }
            else if (b.targetState == "FAN")
            {
                b.targetValue = row.fanCombo->currentText();
            }
            else
            {
                b.targetValue = ""; // ON/OFF/OPEN/CLOSE don't have separate value
            }

            list.append(b);
        }
    }
    return list;
}
