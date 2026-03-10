#ifndef DEVICEITEMWIDGET_H
#define DEVICEITEMWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStyle>

class DeviceItemWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DeviceItemWidget(const QString &name, const QString &type, const QString &status, const QString &id, QWidget *parent = nullptr);
    void setStatus(const QString &status);
    QString getName() const { return m_name; }
    QString getType() const { return m_type; }
    QString getId() const { return m_id; }

signals:
    void controlClicked(const QString &cmd);

private:
    QLabel *m_iconLabel;
    QLabel *m_nameLabel;
    QLabel *m_statusLabel;
    QPushButton *m_controlBtn;
    
    // AC Specific Controls
    QWidget *m_acControlWidget;
    QSpinBox *m_tempSpinBox;
    QComboBox *m_modeCombo;
    QComboBox *m_fanCombo;
    
    // Light Specific Controls
    QWidget *m_lightControlWidget;
    QSpinBox *m_brightnessSpinBox; // 0-100%
    QComboBox *m_colorTempCombo;   // Warm, Neutral, Cold
    
    QString m_name; 
    QString m_id;   
    QString m_type; 
};

#endif // DEVICEITEMWIDGET_H
