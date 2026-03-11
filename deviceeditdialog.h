#ifndef DEVICEEDITDIALOG_H
#define DEVICEEDITDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpinBox>
#include "databasemanager.h"

class DeviceEditDialog : public QDialog
{
    Q_OBJECT
public:
    explicit DeviceEditDialog(QWidget *parent = nullptr);
    void setDeviceData(const DatabaseManager::DeviceData &data);
    DatabaseManager::DeviceData getDeviceData() const;

private slots:
    void onSaveClicked();
    void onCancelClicked();

private:
    void setupUi();

    QLineEdit *m_nameEdit;
    QComboBox *m_typeCombo;
    QSpinBox *m_curtainPositionSpin;
    QLineEdit *m_roomEdit;
    QLineEdit *m_idEdit;
    QPushButton *m_btnSave;
    QPushButton *m_btnCancel;

    DatabaseManager::DeviceData m_data;
};

#endif // DEVICEEDITDIALOG_H
