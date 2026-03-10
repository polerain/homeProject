#include "deviceeditdialog.h"
#include <QMessageBox>

DeviceEditDialog::DeviceEditDialog(QWidget *parent) : QDialog(parent)
{
    setupUi();
    m_data.id = -1; // Default new
}

void DeviceEditDialog::setupUi()
{
    setWindowTitle("编辑设备");
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Name
    QHBoxLayout *nameLayout = new QHBoxLayout();
    nameLayout->addWidget(new QLabel("设备名称:"));
    m_nameEdit = new QLineEdit(this);
    nameLayout->addWidget(m_nameEdit);
    mainLayout->addLayout(nameLayout);

    // Type
    QHBoxLayout *typeLayout = new QHBoxLayout();
    typeLayout->addWidget(new QLabel("设备类型:"));
    m_typeCombo = new QComboBox(this);
    m_typeCombo->addItems({"LIGHT", "AC", "CURTAIN", "FAN"});
    typeLayout->addWidget(m_typeCombo);
    mainLayout->addLayout(typeLayout);

    // Room
    QHBoxLayout *roomLayout = new QHBoxLayout();
    roomLayout->addWidget(new QLabel("所属房间:"));
    m_roomEdit = new QLineEdit(this); // Can use ComboBox if room list is fixed
    m_roomEdit->setPlaceholderText("例如：客厅");
    roomLayout->addWidget(m_roomEdit);
    mainLayout->addLayout(roomLayout);

    // ID
    QHBoxLayout *idLayout = new QHBoxLayout();
    idLayout->addWidget(new QLabel("设备ID:"));
    m_idEdit = new QLineEdit(this);
    m_idEdit->setPlaceholderText("例如：light_living");
    idLayout->addWidget(m_idEdit);
    mainLayout->addLayout(idLayout);

    // Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    m_btnSave = new QPushButton("保存", this);
    m_btnCancel = new QPushButton("取消", this);
    btnLayout->addStretch();
    btnLayout->addWidget(m_btnSave);
    btnLayout->addWidget(m_btnCancel);
    mainLayout->addLayout(btnLayout);

    connect(m_btnSave, &QPushButton::clicked, this, &DeviceEditDialog::onSaveClicked);
    connect(m_btnCancel, &QPushButton::clicked, this, &DeviceEditDialog::onCancelClicked);
}

void DeviceEditDialog::setDeviceData(const DatabaseManager::DeviceData &data)
{
    m_data = data;
    m_nameEdit->setText(data.name);
    m_typeCombo->setCurrentText(data.type);
    m_roomEdit->setText(data.room);
    m_idEdit->setText(data.deviceId);
}

DatabaseManager::DeviceData DeviceEditDialog::getDeviceData() const
{
    DatabaseManager::DeviceData data = m_data;
    data.name = m_nameEdit->text().trimmed();
    data.type = m_typeCombo->currentText();
    data.room = m_roomEdit->text().trimmed();
    data.deviceId = m_idEdit->text().trimmed();
    // Keep original status/params or default
    return data;
}

void DeviceEditDialog::onSaveClicked()
{
    if (m_nameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入设备名称");
        return;
    }
    if (m_idEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入设备ID");
        return;
    }
    accept();
}

void DeviceEditDialog::onCancelClicked()
{
    reject();
}
