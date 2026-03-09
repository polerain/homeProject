#ifndef SCENEWIDGET_H
#define SCENEWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGridLayout>

class SceneWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SceneWidget(QWidget *parent = nullptr);

private slots:
    void onAddClicked();
    void onEditClicked();
    void onDeleteClicked();
    void onActivateClicked();
    void onRefreshClicked();
    void onSceneDoubleClicked(QListWidgetItem *item);

private:
    void setupUi();
    void loadScenes();

    QListWidget *m_sceneList;
    QPushButton *m_btnAdd;
    QPushButton *m_btnEdit;
    QPushButton *m_btnDelete;
    QPushButton *m_btnActivate;
    QPushButton *m_btnRefresh;
};

#endif // SCENEWIDGET_H
