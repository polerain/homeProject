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
        // Ensure item size is sufficient for the custom widget
        item->setSizeHint(QSize(0, 50));

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
        else
        {
            // Generic fallback
            stateCombo->addItems({"ON", "OFF"});
        }

        layout->addWidget(stateCombo);
        layout->addWidget(valSpin);
        layout->addWidget(modeCombo);
        layout->addWidget(fanCombo);

        m_deviceList->setItemWidget(item, widget);

        DeviceRow row;
        row.deviceId = dev.id;
        row.deviceName = dev.name;
        // row.identifier = dev.identifier; // Struct doesn't have identifier yet in DB manager for getAllDevices, assume params or just skip
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
    auto scene = DatabaseManager::instance().getScene(m_sceneId);
    m_nameEdit->setText(scene.name);
    m_descEdit->setText(scene.description);
    m_enabledCheck->setChecked(scene.enabled);

    auto bindings = DatabaseManager::instance().getSceneBindings(m_sceneId);
    for (const auto &bind : bindings)
    {
        for (auto &row : m_rows) // Use reference to modify
        {
            if (row.deviceId == bind.deviceId)
            {
                row.bindCheck->setChecked(true);

                // Find index
                int idx = row.stateCombo->findText(bind.targetState);
                if (idx >= 0)
                    row.stateCombo->setCurrentIndex(idx);

                // Restore value
                if (bind.targetState == "TEMP")
                {
                    row.valueSpin->setValue(bind.targetValue.toInt());
                }
                else if (bind.targetState == "MODE") // Correction: targetState is typically SET_MODE or just MODE depending on implementation.
                // In generateCommand we use targetState as command directly.
                // For AC, if state is MODE, we expect command to be ... wait, generateCommand:
                // if state == "MODE" -> ID_MODE_VAL. So targetState should be "MODE".
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

    // Check name existence if new or name changed
    // Simplified: Just proceed. Or implement checkSceneNameExists in DM.

    return true;
}

DatabaseManager::SceneData SceneEditDialog::getSceneData() const
{
    DatabaseManager::SceneData d;
    d.id = m_sceneId;
    d.name = m_nameEdit->text().trimmed();
    d.description = m_descEdit->toPlainText();
    d.enabled = m_enabledCheck->isChecked();
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
            b.deviceName = row.deviceName;
            // b.identifier = row.identifier; // If we had it
            // We need identifier for execution.
            // In getAllDevices(), we got params. Let's assume params is identifier for now or we fetch it again.
            // Actually, getSceneBindings joins and gets identifier.
            // But here we are creating NEW bindings.
            // We should have stored identifier in DeviceRow.
            // Let's assume we can get it from params or just store it in DeviceRow if we parse params.
            // For now, let's leave identifier empty here, and let SceneExecutor or DatabaseManager handle it?
            // SceneExecutor needs it.
            // DatabaseManager::addScene just stores device_id.
            // DatabaseManager::getSceneBindings JOINS to get identifier.
            // So we DON'T need to provide identifier here for saving!

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
                b.targetValue = "";
            }
            // Delay default 0
            b.delayMs = 0;

            list.append(b);
        }
    }
    return list;
}
