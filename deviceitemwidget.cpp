#include "deviceitemwidget.h"

DeviceItemWidget::DeviceItemWidget(const QString &name, const QString &type, const QString &status, const QString &id, QWidget *parent)
    : QWidget(parent), m_name(name), m_type(type), m_id(id)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(15);

    // Icon placeholder (Circle with color)
    m_iconLabel = new QLabel(this);
    m_iconLabel->setFixedSize(50, 50);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setObjectName("deviceIcon"); // For QSS
    QFont iconFont = m_iconLabel->font();
    iconFont.setPixelSize(24);
    m_iconLabel->setFont(iconFont);

    // Info
    QVBoxLayout *infoLayout = new QVBoxLayout();
    m_nameLabel = new QLabel(name, this);
    m_nameLabel->setObjectName("deviceTitle"); // For QSS
    m_statusLabel = new QLabel(status, this);
    m_statusLabel->setObjectName("deviceStatus"); // For QSS
    infoLayout->addWidget(m_nameLabel);
    infoLayout->addWidget(m_statusLabel);

    // Control Button
    m_controlBtn = new QPushButton(this);
    m_controlBtn->setFixedSize(100, 36);
    m_controlBtn->setObjectName("actionButton"); // For QSS

    if (type == "LIGHT")
    {
        m_iconLabel->setText("💡");
        m_controlBtn->setText(status == "ON" ? "关闭" : "开启");

        // --- Light Extended Controls ---
        m_lightControlWidget = new QWidget(this);
        QHBoxLayout *lightLayout = new QHBoxLayout(m_lightControlWidget);
        lightLayout->setContentsMargins(0, 0, 0, 0);
        lightLayout->setSpacing(5);

        // Brightness
        QLabel *briLabel = new QLabel("亮度:", this);
        m_brightnessSpinBox = new QSpinBox(this);
        m_brightnessSpinBox->setRange(0, 100);
        m_brightnessSpinBox->setValue(80);
        m_brightnessSpinBox->setSuffix("%");
        m_brightnessSpinBox->setMinimumWidth(80);
        m_brightnessSpinBox->setMinimumHeight(30);
        m_brightnessSpinBox->setStyleSheet("QSpinBox { border: 1px solid #ccc; border-radius: 4px; padding: 4px; font-size: 12px; } QSpinBox::up-button, QSpinBox::down-button { width: 20px; height: 15px; }");
        connect(m_brightnessSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int val)
                {
            // LIGHT_LIVING_BRI_80
            emit controlClicked(QString("%1_BRI_%2").arg(m_id.toUpper()).arg(val)); });

        // Color Temp
        m_colorTempCombo = new QComboBox(this);
        m_colorTempCombo->addItems({"暖光", "自然光", "冷白光"});
        m_colorTempCombo->setMinimumWidth(100);
        m_colorTempCombo->setMinimumHeight(30);
        m_colorTempCombo->setStyleSheet("QComboBox { border: 1px solid #ccc; border-radius: 4px; padding: 4px; font-size: 12px; } QComboBox::drop-down { width: 20px; } QComboBox::down-arrow { width: 10px; height: 10px; }");
        connect(m_colorTempCombo, &QComboBox::currentTextChanged, [this](const QString &text)
                {
            // LIGHT_LIVING_COLOR_WARM
            QString color = "NATURAL";
            if(text == "暖光") color = "WARM";
            else if(text == "冷白光") color = "COLD";
            emit controlClicked(QString("%1_COLOR_%2").arg(m_id.toUpper()).arg(color)); });

        lightLayout->addWidget(briLabel);
        lightLayout->addWidget(m_brightnessSpinBox);
        lightLayout->addWidget(m_colorTempCombo);

        infoLayout->addWidget(m_lightControlWidget);

        connect(m_controlBtn, &QPushButton::clicked, [this]()
                {
            QString cmd = (m_statusLabel->text() == "ON") ? "OFF" : "ON";
            // Use ID for command construction: LIGHT_LIVING_ON
            // Expected ID format: light_living -> LIGHT_LIVING
            emit controlClicked(QString("%1_%2").arg(m_id.toUpper()).arg(cmd)); });
    }
    else if (type == "AC")
    {
        m_iconLabel->setText("❄️");
        m_controlBtn->setText(status == "ON" ? "关闭空调" : "开启空调");

        // --- AC Extended Controls ---
        m_acControlWidget = new QWidget(this);
        QHBoxLayout *acLayout = new QHBoxLayout(m_acControlWidget);
        acLayout->setContentsMargins(0, 0, 0, 0);
        acLayout->setSpacing(5);

        // Temperature
        m_tempSpinBox = new QSpinBox(this);
        m_tempSpinBox->setRange(16, 30);
        m_tempSpinBox->setValue(26);
        m_tempSpinBox->setSuffix("°C");
        m_tempSpinBox->setMinimumWidth(90);
        m_tempSpinBox->setMinimumHeight(30);
        m_tempSpinBox->setStyleSheet("QSpinBox { border: 1px solid #ccc; border-radius: 4px; padding: 4px; font-size: 12px; } QSpinBox::up-button, QSpinBox::down-button { width: 20px; height: 15px; }");
        connect(m_tempSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int val)
                {
            // AC_LIVING_TEMP_26
            emit controlClicked(QString("%1_TEMP_%2").arg(m_id.toUpper()).arg(val)); });

        // Mode
        m_modeCombo = new QComboBox(this);
        m_modeCombo->addItems({"制冷", "制暖", "送风", "除湿"});
        m_modeCombo->setMinimumWidth(90);
        m_modeCombo->setMinimumHeight(30);
        m_modeCombo->setStyleSheet("QComboBox { border: 1px solid #ccc; border-radius: 4px; padding: 4px; font-size: 12px; } QComboBox::drop-down { width: 20px; } QComboBox::down-arrow { width: 10px; height: 10px; }");
        connect(m_modeCombo, &QComboBox::currentTextChanged, [this](const QString &text)
                {
            // AC_LIVING_MODE_COOL
            QString mode = "COOL";
            if(text == "制暖") mode = "HEAT";
            else if(text == "送风") mode = "FAN";
            else if(text == "除湿") mode = "DRY";
            emit controlClicked(QString("%1_MODE_%2").arg(m_id.toUpper()).arg(mode)); });

        // Fan Speed
        m_fanCombo = new QComboBox(this);
        m_fanCombo->addItems({"自动", "低风", "中风", "高风"});
        m_fanCombo->setMinimumWidth(90);
        m_fanCombo->setMinimumHeight(30);
        m_fanCombo->setStyleSheet("QComboBox { border: 1px solid #ccc; border-radius: 4px; padding: 4px; font-size: 12px; } QComboBox::drop-down { width: 20px; } QComboBox::down-arrow { width: 10px; height: 10px; }");
        connect(m_fanCombo, &QComboBox::currentTextChanged, [this](const QString &text)
                {
             // AC_LIVING_FAN_AUTO
            QString speed = "AUTO";
            if(text == "低风") speed = "LOW";
            else if(text == "中风") speed = "MID";
            else if(text == "高风") speed = "HIGH";
            emit controlClicked(QString("%1_FAN_%2").arg(m_id.toUpper()).arg(speed)); });

        acLayout->addWidget(m_tempSpinBox);
        acLayout->addWidget(m_modeCombo);
        acLayout->addWidget(m_fanCombo);

        // Add to main layout (insert before button)
        infoLayout->addWidget(m_acControlWidget);

        connect(m_controlBtn, &QPushButton::clicked, [this]()
                {
            QString cmd = (m_statusLabel->text() == "ON") ? "OFF" : "ON";
            emit controlClicked(QString("%1_%2").arg(m_id.toUpper()).arg(cmd)); });
    }
    else if (type == "CURTAIN")
    {
        m_iconLabel->setText("🪟");
        m_controlBtn->setText("开/关窗帘");
        connect(m_controlBtn, &QPushButton::clicked, [this]()
                { emit controlClicked(QString("%1_TOGGLE").arg(m_id.toUpper())); });
    }
    else if (type == "FAN")
    {
        m_iconLabel->setText("🌀");
        m_controlBtn->setText(status == "ON" ? "关闭" : "开启");
        connect(m_controlBtn, &QPushButton::clicked, [this]()
                {
            QString cmd = (m_statusLabel->text() == "ON") ? "OFF" : "ON";
            emit controlClicked(QString("%1_%2").arg(m_id.toUpper()).arg(cmd)); });
    }
    else
    {
        m_controlBtn->setText("操作");
    }

    layout->addWidget(m_iconLabel);
    layout->addLayout(infoLayout);
    layout->addStretch();
    layout->addWidget(m_controlBtn);

    setStatus(status);
}

void DeviceItemWidget::setStatus(const QString &status)
{
    m_statusLabel->setText(status);
    bool isActive = (status == "ON" || status == "OPEN" || status == "100");

    m_iconLabel->setProperty("active", isActive);
    style()->unpolish(m_iconLabel);
    style()->polish(m_iconLabel);

    if (isActive) // 100 means open for curtain
    {
        if (m_type == "LIGHT")
            m_controlBtn->setText("关闭");
        if (m_type == "AC")
            m_controlBtn->setText("关闭空调");
        if (m_type == "CURTAIN")
            m_controlBtn->setText("关闭窗帘");
    }
    else
    {
        if (m_type == "LIGHT")
            m_controlBtn->setText("开启");
        if (m_type == "AC")
            m_controlBtn->setText("开启空调");
        if (m_type == "CURTAIN")
            m_controlBtn->setText("开启窗帘");
    }
}
