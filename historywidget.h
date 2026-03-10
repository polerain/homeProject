#ifndef HISTORYWIDGET_H
#define HISTORYWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTableWidget>
#include <QDateTime>
#include <QTimer>
#include <QTabWidget>
#include <QComboBox>
#include <QDateEdit>
#include <QCheckBox>
#include "qcustomplot.h"
#include "databasemanager.h"

class HistoryWidget : public QWidget
{
    Q_OBJECT
public:
    explicit HistoryWidget(QWidget *parent = nullptr);

public slots:
    void addDataPoint(double temp, double humidity);

private slots:
    void onExportClicked();
    void onQueryLogsClicked();
    void onQueryEnvClicked();
    void onClearLogsClicked();
    void onClearEnvClicked();
    void onTabChanged(int index);
    void onDataReceived(const QString &data);

private:
    void setupUi();
    void setupChart();

    // UI Components
    QTabWidget *m_tabWidget;

    // Log Tab
    QWidget *m_logTab;
    QTableWidget *m_logTable;
    QComboBox *m_logDeviceTypeCombo;
    QComboBox *m_logActionTypeCombo;
    QDateEdit *m_logStartDate;
    QDateEdit *m_logEndDate;
    QPushButton *m_logQueryBtn;
    QPushButton *m_logExportBtn;
    QPushButton *m_logClearBtn;

    // Env Tab
    QWidget *m_envTab;
    QCustomPlot *m_plot;
    QTableWidget *m_envTable;
    QPushButton *m_envQueryBtn;
    QPushButton *m_envExportBtn;
    QPushButton *m_envClearBtn;
    QCheckBox *m_showTempCheck;
    QCheckBox *m_showHumidCheck;

    void loadLogs();
    void loadEnvData();
};

#endif // HISTORYWIDGET_H
