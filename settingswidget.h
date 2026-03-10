#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include <QLineEdit>
#include <QGroupBox>

class SettingsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SettingsWidget(QWidget *parent = nullptr);

private slots:
    void onBackupClicked();
    void onRestoreClicked();
    void onSaveSettingsClicked();

private:
    void setupUi();
    void loadSettings();

    QComboBox *m_themeCombo;
    QCheckBox *m_alarmSoundCheck;
    QComboBox *m_refreshRateCombo;
    QPushButton *m_btnBackup;
    QPushButton *m_btnRestore;
    QPushButton *m_btnSave;
};

#endif // SETTINGSWIDGET_H
