#ifndef SCENEITEMWIDGET_H
#define SCENEITEMWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "databasemanager.h"

class SceneItemWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SceneItemWidget(const DatabaseManager::SceneData &scene, const QString &bindingsSummary, QWidget *parent = nullptr);
    int getSceneId() const { return m_sceneId; }

private:
    int m_sceneId;
    QLabel *m_nameLabel;
    QLabel *m_descLabel;
    QLabel *m_bindingsLabel;
    QLabel *m_statusLabel;
};

#endif // SCENEITEMWIDGET_H
