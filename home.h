#ifndef HOME_H
#define HOME_H

#include <QWidget>
#include <QStackedWidget>
#include "loginwidget.h"
#include "dashboardwidget.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class home;
}
QT_END_NAMESPACE

class home : public QWidget
{
    Q_OBJECT

public:
    home(QWidget *parent = nullptr);
    ~home();

private:
    Ui::home *ui;
    
    QStackedWidget *m_mainStack;
    LoginWidget *m_loginWidget;
    DashboardWidget *m_dashboardWidget;
    
    void initUI();
};
#endif // HOME_H
