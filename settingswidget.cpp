#include "settingswidget.h"
#include "databasemanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QApplication>
#include <QStyleFactory>

SettingsWidget::SettingsWidget(QWidget *parent) : QWidget(parent)
{
    setupUi();
    loadSettings();
}

void SettingsWidget::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 30, 30, 30);

    QLabel *title = new QLabel("系统设置", this);
    QFont f = title->font();
    f.setPointSize(16);
    f.setBold(true);
    title->setFont(f);
    title->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(title);

    // 1. General Settings
    QGroupBox *generalGroup = new QGroupBox("常规设置", this);
    QVBoxLayout *generalLayout = new QVBoxLayout(generalGroup);

    // Theme
    QHBoxLayout *themeLayout = new QHBoxLayout();
    themeLayout->addWidget(new QLabel("界面主题:"));
    m_themeCombo = new QComboBox(this);
    m_themeCombo->addItems({"默认", "浅色", "深色"}); // Placeholder for theme logic
    themeLayout->addWidget(m_themeCombo);
    themeLayout->addStretch();
    generalLayout->addLayout(themeLayout);

    // Alarm Sound
    QHBoxLayout *soundLayout = new QHBoxLayout();
    m_alarmSoundCheck = new QCheckBox("开启报警提示音", this);
    soundLayout->addWidget(m_alarmSoundCheck);
    soundLayout->addStretch();
    generalLayout->addLayout(soundLayout);

    // Refresh Rate
    QHBoxLayout *rateLayout = new QHBoxLayout();
    rateLayout->addWidget(new QLabel("数据刷新频率:"));
    m_refreshRateCombo = new QComboBox(this);
    m_refreshRateCombo->addItems({"1秒", "3秒", "5秒", "10秒"});
    rateLayout->addWidget(m_refreshRateCombo);
    rateLayout->addStretch();
    generalLayout->addLayout(rateLayout);

    m_btnSave = new QPushButton("保存设置", this);
    m_btnSave->setFixedWidth(100);
    QHBoxLayout *saveLayout = new QHBoxLayout();
    saveLayout->addStretch();
    saveLayout->addWidget(m_btnSave);
    generalLayout->addLayout(saveLayout);

    mainLayout->addWidget(generalGroup);

    // 2. Database Management
    QGroupBox *dbGroup = new QGroupBox("数据管理", this);
    QVBoxLayout *dbLayout = new QVBoxLayout(dbGroup);

    QHBoxLayout *backupLayout = new QHBoxLayout();
    m_btnBackup = new QPushButton("备份数据库", this);
    m_btnRestore = new QPushButton("恢复数据库", this);
    backupLayout->addWidget(m_btnBackup);
    backupLayout->addWidget(m_btnRestore);
    backupLayout->addStretch();
    
    QLabel *tipLabel = new QLabel("提示: 恢复数据库会覆盖当前所有数据，请谨慎操作。", this);
    tipLabel->setStyleSheet("color: gray; font-size: 12px;");

    dbLayout->addLayout(backupLayout);
    dbLayout->addWidget(tipLabel);

    mainLayout->addWidget(dbGroup);
    mainLayout->addStretch();

    // Connects
    connect(m_btnBackup, &QPushButton::clicked, this, &SettingsWidget::onBackupClicked);
    connect(m_btnRestore, &QPushButton::clicked, this, &SettingsWidget::onRestoreClicked);
    connect(m_btnSave, &QPushButton::clicked, this, &SettingsWidget::onSaveSettingsClicked);
}

void SettingsWidget::loadSettings()
{
    QSettings settings("MyCompany", "SmartHome");
    QString theme = settings.value("theme", "默认").toString();
    m_themeCombo->setCurrentText(theme);

    bool sound = settings.value("alarmSound", true).toBool();
    m_alarmSoundCheck->setChecked(sound);

    QString rate = settings.value("refreshRate", "3秒").toString();
    m_refreshRateCombo->setCurrentText(rate);
}

void SettingsWidget::onSaveSettingsClicked()
{
    QSettings settings("MyCompany", "SmartHome");
    settings.setValue("theme", m_themeCombo->currentText());
    settings.setValue("alarmSound", m_alarmSoundCheck->isChecked());
    settings.setValue("refreshRate", m_refreshRateCombo->currentText());
    
    QMessageBox::information(this, "成功", "设置已保存");
    
    // Apply Theme (Simple Example)
    if (m_themeCombo->currentText() == "深色") {
        // Apply dark style (placeholder)
        // qApp->setStyleSheet("QWidget { background-color: #333; color: white; }");
    } else {
        // qApp->setStyleSheet(""); // Reset
    }
}

void SettingsWidget::onBackupClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "备份数据库", "backup.db", "Database Files (*.db)");
    if (fileName.isEmpty()) return;

    if (DatabaseManager::instance().backupDatabase(fileName)) {
        QMessageBox::information(this, "成功", "数据库备份成功");
    } else {
        QMessageBox::critical(this, "失败", "数据库备份失败");
    }
}

void SettingsWidget::onRestoreClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "恢复数据库", "", "Database Files (*.db)");
    if (fileName.isEmpty()) return;

    if (QMessageBox::warning(this, "警告", "恢复操作将覆盖当前所有数据，且不可撤销。\n确定要继续吗？", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        if (DatabaseManager::instance().restoreDatabase(fileName)) {
            QMessageBox::information(this, "成功", "数据库恢复成功，请重启应用以确保数据正常加载。");
        } else {
            QMessageBox::critical(this, "失败", "数据库恢复失败");
        }
    }
}
