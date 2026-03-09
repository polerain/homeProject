#ifndef HISTORYWIDGET_H
#define HISTORYWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTableWidget>
#include <QDateTime>
#include <QTimer>
#include "qcustomplot.h"

class HistoryWidget : public QWidget
{
    Q_OBJECT
public:
    explicit HistoryWidget(QWidget *parent = nullptr);

public slots:
    void addDataPoint(double temp, double humidity);

private slots:
    void onExportClicked();

private:
    void setupUi();
    void setupChart();

    QCustomPlot *m_plot;
    QTableWidget *m_table;
    QPushButton *m_exportBtn;
};

#endif // HISTORYWIDGET_H
