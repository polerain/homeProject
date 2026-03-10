#ifndef AIASSISTANTWIDGET_H
#define AIASSISTANTWIDGET_H

#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFrame>
#include "weathermanager.h"

class AIAssistantWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AIAssistantWidget(QWidget* parent = nullptr);

private slots:
    void onQueryWeather();
    void onWeatherReceived(const WeatherManager::WeatherData& weather);
    void onWeatherErrorOccurred(const QString& message);

private:
    void setupUi();
    
    QLineEdit* m_locationEdit;
    QPushButton* m_weatherBtn;
    
    QLabel* m_weatherConditionLabel;
    QLabel* m_weatherTemperatureLabel;
    QLabel* m_weatherHumidityLabel;
    QLabel* m_weatherWindLabel;
    
    QLabel* m_weatherClothingLabel;
    QLabel* m_weatherHomeSettingsLabel;
    QLabel* m_weatherAdviceLabel;
    
    QFrame* m_weatherCard;
};

#endif // AIASSISTANTWIDGET_H
