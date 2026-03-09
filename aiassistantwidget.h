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
#include "aiassistant.h"

class AIAssistantWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AIAssistantWidget(QWidget* parent = nullptr);

private slots:
    void onGetSuggestions();
    void onSuggestionsReceived(const AIAssistant::WeatherData& weather, const AIAssistant::Suggestions& suggestions);
    void onErrorOccurred(const QString& message);

private:
    void setupUi();
    
    QLineEdit* m_locationEdit;
    QPushButton* m_getBtn;
    
    QLabel* m_temperatureLabel;
    QLabel* m_conditionLabel;
    QLabel* m_humidityLabel;
    QLabel* m_windLabel;
    
    QLabel* m_clothingLabel;
    QLabel* m_homeSettingsLabel;
    QLabel* m_adviceLabel;
};

#endif // AIASSISTANTWIDGET_H
