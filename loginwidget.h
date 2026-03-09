#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QMessageBox>

class LoginWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LoginWidget(QWidget *parent = nullptr);

signals:
    void loginSuccessful();

private slots:
    void onLoginClicked();

private:
    QLineEdit *m_usernameEdit;
    QLineEdit *m_passwordEdit;
    QPushButton *m_loginBtn;
    QPushButton *m_registerBtn;

    void setupUi();
};

#endif // LOGINWIDGET_H
