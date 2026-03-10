#include "historywidget.h"
#include "tcpmanager.h"
#include <QHeaderView>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QLabel>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

HistoryWidget::HistoryWidget(QWidget *parent) : QWidget(parent)
{
    setupUi();
    setupChart();
    // Default load some data? Or wait for user query.
    // Let's set default dates to today
    m_logStartDate->setDate(QDate::currentDate());
    m_logEndDate->setDate(QDate::currentDate());
}

void HistoryWidget::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_tabWidget = new QTabWidget(this);

    // --- Tab 1: Operation Logs ---
    m_logTab = new QWidget();
    QVBoxLayout *logLayout = new QVBoxLayout(m_logTab);

    // Filters
    QGroupBox *logFilterGroup = new QGroupBox("筛选条件");
    QHBoxLayout *logFilterLayout = new QHBoxLayout(logFilterGroup);

    logFilterLayout->addWidget(new QLabel("时间范围:"));
    m_logStartDate = new QDateEdit(this);
    m_logStartDate->setCalendarPopup(true);
    logFilterLayout->addWidget(m_logStartDate);
    logFilterLayout->addWidget(new QLabel("-"));
    m_logEndDate = new QDateEdit(this);
    m_logEndDate->setCalendarPopup(true);
    logFilterLayout->addWidget(m_logEndDate);

    logFilterLayout->addWidget(new QLabel("动作类型:"));
    m_logActionTypeCombo = new QComboBox(this);
    m_logActionTypeCombo->addItems({"全部", "登录", "设备控制", "场景执行", "报警"});
    logFilterLayout->addWidget(m_logActionTypeCombo);

    // logFilterLayout->addWidget(new QLabel("设备名称:")); // Simplified filter
    // m_logDeviceTypeCombo = new QComboBox(this);
    // logFilterLayout->addWidget(m_logDeviceTypeCombo);

    m_logQueryBtn = new QPushButton("查询", this);
    m_logExportBtn = new QPushButton("导出", this);
    m_logClearBtn = new QPushButton("清空", this);
    m_logClearBtn->setStyleSheet("background-color: #dc3545; color: white;");
    logFilterLayout->addWidget(m_logQueryBtn);
    logFilterLayout->addWidget(m_logExportBtn);
    logFilterLayout->addWidget(m_logClearBtn);
    logFilterLayout->addStretch();

    logLayout->addWidget(logFilterGroup);

    // Table
    m_logTable = new QTableWidget(this);
    m_logTable->setColumnCount(5);
    m_logTable->setHorizontalHeaderLabels({"时间", "用户", "设备", "操作", "结果"});
    m_logTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_logTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    logLayout->addWidget(m_logTable);

    m_tabWidget->addTab(m_logTab, "操作日志");

    // --- Tab 2: Environment Data ---
    m_envTab = new QWidget();
    QVBoxLayout *envLayout = new QVBoxLayout(m_envTab);

    // Filters
    QGroupBox *envFilterGroup = new QGroupBox("操作");
    QHBoxLayout *envFilterLayout = new QHBoxLayout(envFilterGroup);

    m_envQueryBtn = new QPushButton("获取最新模拟数据", this);
    m_envExportBtn = new QPushButton("导出", this);
    m_envClearBtn = new QPushButton("清空", this);
    m_envClearBtn->setStyleSheet("background-color: #dc3545; color: white;");
    envFilterLayout->addWidget(m_envQueryBtn);
    envFilterLayout->addWidget(m_envExportBtn);
    envFilterLayout->addWidget(m_envClearBtn);

    envFilterLayout->addStretch();

    envLayout->addWidget(envFilterGroup);

    // Chart
    m_plot = new QCustomPlot(this);
    m_plot->setMinimumHeight(300);
    envLayout->addWidget(m_plot);

    // Table (Optional, or toggle view)
    m_envTable = new QTableWidget(this);
    m_envTable->setColumnCount(3);
    m_envTable->setHorizontalHeaderLabels({"时间", "温度(°C)", "湿度(%)"});
    m_envTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_envTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    envLayout->addWidget(m_envTable);

    m_tabWidget->addTab(m_envTab, "环境数据");

    mainLayout->addWidget(m_tabWidget);

    // Connects
    connect(m_logQueryBtn, &QPushButton::clicked, this, &HistoryWidget::onQueryLogsClicked);
    connect(m_logExportBtn, &QPushButton::clicked, this, &HistoryWidget::onExportClicked);
    connect(m_logClearBtn, &QPushButton::clicked, this, &HistoryWidget::onClearLogsClicked);

    connect(m_envQueryBtn, &QPushButton::clicked, this, &HistoryWidget::onQueryEnvClicked);
    connect(m_envExportBtn, &QPushButton::clicked, this, &HistoryWidget::onExportClicked);
    connect(m_envClearBtn, &QPushButton::clicked, this, &HistoryWidget::onClearEnvClicked);

    connect(m_tabWidget, &QTabWidget::currentChanged, this, &HistoryWidget::onTabChanged);

    // Listen for data from server
    connect(&TcpManager::instance(), &TcpManager::dataReceived, this, &HistoryWidget::onDataReceived);
}

void HistoryWidget::setupChart()
{
    // Add two graphs: one for Temp, one for Humidity
    m_plot->addGraph();
    m_plot->graph(0)->setPen(QPen(Qt::red)); // Temp is Red
    m_plot->graph(0)->setName("温度");

    m_plot->addGraph();
    m_plot->graph(1)->setPen(QPen(Qt::blue)); // Humidity is Blue
    m_plot->graph(1)->setName("湿度");

    // X Axis: Time
    QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);
    dateTicker->setDateTimeFormat("MM-dd HH:mm");
    m_plot->xAxis->setTicker(dateTicker);
    m_plot->xAxis->setLabel("时间");

    // Y Axis
    m_plot->yAxis->setLabel("数值");
    m_plot->yAxis->setRange(0, 100);

    m_plot->legend->setVisible(true);
    // m_plot->replot();
}

void HistoryWidget::addDataPoint(double temp, double humidity)
{
    // Save to DB
    DatabaseManager::instance().addEnvData(temp, humidity);

    // If currently viewing live mode, update chart/table?
    // Since we now use simulated history, we might not auto-refresh unless requested.
    // Or we can keep live update if we are not looking at history.
    // For now, let's just save to DB.
}

void HistoryWidget::onTabChanged(int index)
{
    if (index == 0)
    {
        // Log tab active
    }
    else
    {
        // Env tab active
    }
}

void HistoryWidget::onQueryLogsClicked()
{
    QString start = m_logStartDate->date().toString("yyyy-MM-dd") + " 00:00:00";
    QString end = m_logEndDate->date().toString("yyyy-MM-dd") + " 23:59:59";
    QString filterType = m_logActionTypeCombo->currentText();

    // 获取所有日志，然后在内存中筛选
    auto logs = DatabaseManager::instance().getLogs("", start, end, "");

    m_logTable->setRowCount(0);
    for (const auto &log : logs)
    {
        bool match = true;

        if (filterType == "全部")
        {
            match = true;
        }
        else if (filterType == "登录")
        {
            match = log.action.contains("登录");
        }
        else if (filterType == "设备控制")
        {
            // 排除场景和AI，剩下的认为是设备控制
            match = !log.deviceName.startsWith("场景") && !log.deviceName.startsWith("AI助手") && !log.action.contains("登录") && !log.action.contains("报警");
        }
        else if (filterType == "场景执行")
        {
            match = log.deviceName.startsWith("场景") || log.action.contains("执行场景");
        }
        else if (filterType == "报警")
        {
            match = log.action.contains("报警");
        }

        if (match)
        {
            int row = m_logTable->rowCount();
            m_logTable->insertRow(row);
            m_logTable->setItem(row, 0, new QTableWidgetItem(log.timestamp));
            m_logTable->setItem(row, 1, new QTableWidgetItem(log.user));
            m_logTable->setItem(row, 2, new QTableWidgetItem(log.deviceName));
            m_logTable->setItem(row, 3, new QTableWidgetItem(log.action));
            m_logTable->setItem(row, 4, new QTableWidgetItem(log.result));
        }
    }
}

void HistoryWidget::onQueryEnvClicked()
{
    // Clear current data
    m_envTable->setRowCount(0);
    m_plot->graph(0)->data()->clear();
    m_plot->graph(1)->data()->clear();
    m_plot->replot();

    // Send command to server
    if (TcpManager::instance().isConnected())
    {
        TcpManager::instance().sendCommand("GET_HISTORY_ENV_DATA");
    }
    else
    {
        QMessageBox::warning(this, "提示", "未连接到设备服务器，无法获取模拟数据");
    }
}

void HistoryWidget::onDataReceived(const QString &data)
{
    if (data.startsWith("HISTORY_ENV_DATA:"))
    {
        QString jsonStr = data.mid(17); // Length of "HISTORY_ENV_DATA:"

        QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
        if (!doc.isArray())
            return;

        QJsonArray array = doc.array();

        // Update Table
        m_envTable->setRowCount(0);

        // Update Chart
        m_plot->graph(0)->data()->clear();
        m_plot->graph(1)->data()->clear();

        for (const auto &val : array)
        {
            QJsonObject obj = val.toObject();
            QString timestamp = obj["timestamp"].toString();
            double temp = obj["temperature"].toDouble();
            double humid = obj["humidity"].toDouble();

            // Table
            int row = m_envTable->rowCount();
            m_envTable->insertRow(row);
            m_envTable->setItem(row, 0, new QTableWidgetItem(timestamp));
            m_envTable->setItem(row, 1, new QTableWidgetItem(QString::number(temp, 'f', 1)));
            m_envTable->setItem(row, 2, new QTableWidgetItem(QString::number(humid, 'f', 1)));

            // Chart
            QDateTime dt = QDateTime::fromString(timestamp, "yyyy-MM-dd HH:mm:ss");
            double key = dt.toSecsSinceEpoch();
            m_plot->graph(0)->addData(key, temp);
            m_plot->graph(1)->addData(key, humid);
        }

        // Rescale
        m_plot->rescaleAxes();
        m_plot->yAxis->setRange(0, 100);
        m_plot->replot();
    }
}

void HistoryWidget::onClearLogsClicked()
{
    if (QMessageBox::question(this, "确认", "确定要清空所有操作日志吗？此操作不可恢复。") == QMessageBox::Yes)
    {
        if (DatabaseManager::instance().clearLogs())
        {
            QMessageBox::information(this, "成功", "日志已清空");
            m_logTable->setRowCount(0);
        }
        else
        {
            QMessageBox::critical(this, "失败", "清空日志失败");
        }
    }
}

void HistoryWidget::onClearEnvClicked()
{
    if (QMessageBox::question(this, "确认", "确定要清空所有环境数据吗？此操作不可恢复。") == QMessageBox::Yes)
    {
        if (DatabaseManager::instance().clearEnvData())
        {
            QMessageBox::information(this, "成功", "环境数据已清空");
            m_envTable->setRowCount(0);
            m_plot->graph(0)->data()->clear();
            m_plot->graph(1)->data()->clear();
            m_plot->replot();
        }
        else
        {
            QMessageBox::critical(this, "失败", "清空数据失败");
        }
    }
}

void HistoryWidget::onExportClicked()
{
    bool isLogTab = (m_tabWidget->currentWidget() == m_logTab);
    QTableWidget *table = isLogTab ? m_logTable : m_envTable;

    if (table->rowCount() == 0)
    {
        QMessageBox::information(this, "提示", "当前没有数据可导出");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, "导出数据", "", "CSV Files (*.csv)");
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file);

        // Headers
        QStringList headers;
        for (int i = 0; i < table->columnCount(); ++i)
        {
            headers << table->horizontalHeaderItem(i)->text();
        }
        out << headers.join(",") << "\n";

        // Rows
        for (int i = 0; i < table->rowCount(); ++i)
        {
            QStringList rowData;
            for (int j = 0; j < table->columnCount(); ++j)
            {
                rowData << table->item(i, j)->text();
            }
            out << rowData.join(",") << "\n";
        }
        file.close();
        QMessageBox::information(this, "成功", "数据导出成功！");
    }
    else
    {
        QMessageBox::critical(this, "错误", "无法写入文件！");
    }
}
