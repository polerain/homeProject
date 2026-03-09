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
    void onActivateScene(QListWidgetItem *item);

private:
    void setupUi();
    void loadScenes();
    
    QListWidget *m_sceneList;
};

#endif // SCENEWIDGET_H
