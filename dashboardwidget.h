#ifndef DASHBOARDWIDGET_H
#define DASHBOARDWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QStackedWidget>
#include <QListWidget>

class DashboardWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DashboardWidget(QWidget *parent = nullptr);

signals:
    void logoutRequested();

private:
    void setupUi();
    void setupSidebar();
    void setupContentArea();
    
    QListWidget *m_menuList;
    QStackedWidget *m_contentStack;
};

#endif // DASHBOARDWIDGET_H
