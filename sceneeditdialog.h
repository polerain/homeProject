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
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include "scenemodels.h"
#include "devicemanager.h"

// Item widget for device configuration in the list
class SceneDeviceItemWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SceneDeviceItemWidget(const DeviceInfo &device, QWidget *parent = nullptr);

    void setBinding(const SceneDeviceBinding &binding);
    SceneDeviceBinding getBinding() const;
    bool isSelected() const;

private:
    DeviceInfo m_device;
    QCheckBox *m_selectCb;
    QLabel *m_nameLabel;
    QWidget *m_controlArea;

    // Light controls
    QComboBox *m_powerCombo;
    QSpinBox *m_brightnessSpin;
    QComboBox *m_lightModeCombo;

    // AC controls
    QSpinBox *m_tempSpin;
    QComboBox *m_acModeCombo;
    QComboBox *m_fanCombo;

    // Curtain controls
    QComboBox *m_curtainActionCombo;
};

class SceneEditDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SceneEditDialog(QWidget *parent = nullptr);

    void setSceneInfo(const SceneInfo &info);
    SceneInfo getSceneInfo() const;

    void setBindings(const QList<SceneDeviceBinding> &bindings);
    QList<SceneDeviceBinding> getBindings() const;

private slots:
    void onSave();

private:
    void setupUi();
    void loadDevices();

    QLineEdit *m_nameEdit;
    QLineEdit *m_descEdit;
    QCheckBox *m_enabledCb;
    QListWidget *m_deviceList;

    SceneInfo m_sceneInfo;
};

#endif // SCENEEDITDIALOG_H
