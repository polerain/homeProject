#include "historywidget.h"
#include <QHeaderView>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>

HistoryWidget::HistoryWidget(QWidget *parent) : QWidget(parent)
{
    setupUi();
    setupChart();
}

void HistoryWidget::setupUi()
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    // Top: Chart
    m_plot = new QCustomPlot(this);
    m_plot->setMinimumHeight(300);
    layout->addWidget(m_plot);

    // Middle: Buttons
    m_exportBtn = new QPushButton("导出数据", this);
    layout->addWidget(m_exportBtn);

    // Bottom: Table
    m_table = new QTableWidget(this);
    m_table->setColumnCount(3);
    m_table->setHorizontalHeaderLabels({"时间", "温度(°C)", "湿度(%)"});
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    layout->addWidget(m_table);

    connect(m_exportBtn, &QPushButton::clicked, this, &HistoryWidget::onExportClicked);
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
    dateTicker->setDateTimeFormat("HH:mm:ss");
    m_plot->xAxis->setTicker(dateTicker);
    m_plot->xAxis->setLabel("时间");

    // Y Axis
    m_plot->yAxis->setLabel("数值");
    m_plot->yAxis->setRange(0, 100);

    m_plot->legend->setVisible(true);
    m_plot->replot();
}

void HistoryWidget::addDataPoint(double temp, double humidity)
{
    double now = QDateTime::currentDateTime().toSecsSinceEpoch();

    // Add to chart
    m_plot->graph(0)->addData(now, temp);
    m_plot->graph(1)->addData(now, humidity);

    // Rescale axes automatically
    m_plot->xAxis->setRange(now, 60, Qt::AlignRight); // Show last 60 seconds (scrolling)
    m_plot->replot();

    // Add to table
    int row = m_table->rowCount();
    m_table->insertRow(row);
    m_table->setItem(row, 0, new QTableWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")));
    m_table->setItem(row, 1, new QTableWidgetItem(QString::number(temp, 'f', 1)));
    m_table->setItem(row, 2, new QTableWidgetItem(QString::number(humidity, 'f', 1)));

    // Auto scroll table
    m_table->scrollToBottom();
}

void HistoryWidget::onExportClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "导出数据", "", "CSV Files (*.csv)");
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file);
        out << "Time,Temperature,Humidity\n";
        for (int i = 0; i < m_table->rowCount(); ++i)
        {
            out << m_table->item(i, 0)->text() << ","
                << m_table->item(i, 1)->text() << ","
                << m_table->item(i, 2)->text() << "\n";
        }
        file.close();
        QMessageBox::information(this, "成功", "数据导出成功！");
    }
    else
    {
        QMessageBox::critical(this, "错误", "无法写入文件！");
    }
}
