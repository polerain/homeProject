#ifndef SCENEWIDGET_H
#define SCENEWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include "deviceitemwidget.h"
#include "devicemanager.h"
#include "databasemanager.h"

class SceneWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SceneWidget(QWidget *parent = nullptr);

private slots:
    void onActivateScene(QListWidgetItem *item);
    void onSceneSelected(QListWidgetItem *item);
    void onDeviceControl(const QString &cmd);

    // CRUD Slots
    void onAddScene();
    void onEditScene();
    void onDeleteScene();
    void onActivateBtnClicked();

private:
    void setupUi();
    void loadScenes();
    void loadSceneDevices(int sceneId);
    void executeScene(int sceneId);
    // QString generateCommand(const SceneDeviceBinding &binding);

    QListWidget *m_sceneList;
    QListWidget *m_deviceList;
    QLabel *m_sceneDetailLabel;

    // Buttons
    QPushButton *m_addBtn;
    QPushButton *m_editBtn;
    QPushButton *m_delBtn;
    QPushButton *m_activateBtn;
};

#endif // SCENEWIDGET_H
