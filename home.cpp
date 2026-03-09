#include "home.h"
#include "ui_home.h"
#include "databasemanager.h"
#include <QVBoxLayout>

home::home(QWidget *parent)
    : QWidget(parent), ui(new Ui::home)
{
    ui->setupUi(this);

    // Initialize Database
    DatabaseManager::instance().openDatabase();

    initUI();
}

home::~home()
{
    delete ui;
}

void home::initUI()
{
    // Use a layout for the main window if not already present in UI
    if (!layout())
    {
        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        setLayout(layout);
    }

    m_mainStack = new QStackedWidget(this);

    m_loginWidget = new LoginWidget(this);
    m_dashboardWidget = new DashboardWidget(this);

    m_mainStack->addWidget(m_loginWidget);     // Index 0
    m_mainStack->addWidget(m_dashboardWidget); // Index 1

    // Add stack to main layout
    layout()->addWidget(m_mainStack);

    // Connect Login Signal
    connect(m_loginWidget, &LoginWidget::loginSuccessful, [this]()
            { m_mainStack->setCurrentWidget(m_dashboardWidget); });

    // Connect Logout Signal (optional)
    // connect(m_dashboardWidget, &DashboardWidget::logoutRequested, [this](){
    //     m_mainStack->setCurrentWidget(m_loginWidget);
    // });

    // Show Login initially
    m_mainStack->setCurrentWidget(m_loginWidget);
}
