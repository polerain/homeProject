#include "loginwidget.h"
#include "databasemanager.h"

LoginWidget::LoginWidget(QWidget *parent) : QWidget(parent)
{
    setupUi();
}

void LoginWidget::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Title
    QLabel *titleLabel = new QLabel("智能家居监控平台", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont font = titleLabel->font();
    font.setPointSize(20);
    font.setBold(true);
    titleLabel->setFont(font);
    
    // Form Layout
    QWidget *formWidget = new QWidget(this);
    QGridLayout *formLayout = new QGridLayout(formWidget);
    QLabel *userLabel = new QLabel("用户名:", this);
    QLabel *passLabel = new QLabel("密  码:", this);
    
    m_usernameEdit = new QLineEdit(this);
    m_usernameEdit->setPlaceholderText("请输入用户名");
    
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText("请输入密码");
    
    formLayout->addWidget(userLabel, 0, 0);
    formLayout->addWidget(m_usernameEdit, 0, 1);
    formLayout->addWidget(passLabel, 1, 0);
    formLayout->addWidget(m_passwordEdit, 1, 1);
    
    // Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    m_loginBtn = new QPushButton("登录", this);
    m_registerBtn = new QPushButton("注册", this);
    
    btnLayout->addWidget(m_loginBtn);
    btnLayout->addWidget(m_registerBtn);
    
    mainLayout->addStretch();
    mainLayout->addWidget(titleLabel);
    mainLayout->addSpacing(20);
    mainLayout->addWidget(formWidget);
    mainLayout->addSpacing(20);
    mainLayout->addLayout(btnLayout);
    mainLayout->addStretch();
    
    // Signals
    connect(m_loginBtn, &QPushButton::clicked, this, &LoginWidget::onLoginClicked);
}

void LoginWidget::onLoginClicked()
{
    QString username = m_usernameEdit->text();
    QString password = m_passwordEdit->text();
    
    if (username.isEmpty()) {
        QMessageBox::warning(this, "提示", "用户名不能为空");
        return;
    }
    
    // TODO: Verify with DatabaseManager
    // For now, simple emit
    emit loginSuccessful();
}
