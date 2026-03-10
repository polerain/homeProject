#ifndef SCENEEDITDIALOG_H
#define SCENEEDITDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QListWidget>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include "databasemanager.h"

class SceneEditDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SceneEditDialog(QWidget *parent = nullptr, int sceneId = -1);
    
    DatabaseManager::SceneData getSceneData() const;
    QList<DatabaseManager::SceneDeviceBinding> getBindings() const;

private:
    void setupUi();
    void loadData();
    void populateDeviceList();
    bool validate();

    int m_sceneId;
    QLineEdit *m_nameEdit;
    QTextEdit *m_descEdit;
    QCheckBox *m_enabledCheck;
    QListWidget *m_deviceList;
    
    // Helper to store row widgets for retrieval
    struct DeviceRow {
        int deviceId;
        QString deviceName;
        QString identifier;
        QString type;
        
        QCheckBox *bindCheck;
        // Depending on type, different widgets visible
        QComboBox *stateCombo; // For ON/OFF/OPEN/CLOSE
        QSpinBox *valueSpin;   // For Temp
        QComboBox *modeCombo;  // For Mode
        QComboBox *fanCombo;   // For Fan
    };
    QList<DeviceRow> m_rows;
};

#endif // SCENEEDITDIALOG_H
