#include "sceneitemwidget.h"
#include <QFrame>

SceneItemWidget::SceneItemWidget(const DatabaseManager::SceneData &scene, const QString &bindingsSummary, QWidget *parent)
    : QWidget(parent), m_sceneId(scene.id)
{
    // Styling similar to DeviceItemWidget or Card style
    // Main horizontal layout
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(15);

    // Icon
    QLabel *iconLabel = new QLabel("🎬"); // Emoji as placeholder icon
    QFont iconFont = iconLabel->font();
    iconFont.setPixelSize(32);
    iconLabel->setFont(iconFont);
    iconLabel->setFixedSize(50, 50);
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet("background-color: #e0e0e0; border-radius: 25px;");

    mainLayout->addWidget(iconLabel);

    // Info Column (Name, Desc)
    QVBoxLayout *infoLayout = new QVBoxLayout();
    m_nameLabel = new QLabel(scene.name);
    m_nameLabel->setStyleSheet("font-weight: bold; font-size: 16px;");

    m_descLabel = new QLabel(scene.description.isEmpty() ? "暂无描述" : scene.description);
    m_descLabel->setStyleSheet("color: #666; font-size: 12px;");

    infoLayout->addWidget(m_nameLabel);
    infoLayout->addWidget(m_descLabel);
    infoLayout->addStretch();

    mainLayout->addLayout(infoLayout, 1); // Stretch factor 1

    // Bindings Summary Column
    QVBoxLayout *bindingLayout = new QVBoxLayout();
    QLabel *bindingTitle = new QLabel("设备设置:");
    bindingTitle->setStyleSheet("font-weight: bold; color: #555; font-size: 12px;");
    m_bindingsLabel = new QLabel(bindingsSummary);
    m_bindingsLabel->setStyleSheet("color: #333; font-size: 12px;");
    m_bindingsLabel->setWordWrap(true);
    m_bindingsLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft); // Align top for multiline

    bindingLayout->addWidget(bindingTitle);
    bindingLayout->addWidget(m_bindingsLabel);
    bindingLayout->addStretch();

    mainLayout->addLayout(bindingLayout, 3); // Stretch factor 3 (increased width)

    // Status Column
    QVBoxLayout *statusLayout = new QVBoxLayout();
    m_statusLabel = new QLabel(scene.enabled ? "已启用" : "已禁用");
    m_statusLabel->setStyleSheet(scene.enabled ? "color: #28a745; font-weight: bold; border: 1px solid #28a745; border-radius: 4px; padding: 2px 5px;" : "color: #dc3545; font-weight: bold; border: 1px solid #dc3545; border-radius: 4px; padding: 2px 5px;");
    m_statusLabel->setAlignment(Qt::AlignCenter);

    // QLabel *lastActiveLabel = new QLabel(scene.lastActivated.isEmpty() ? "从未激活" : scene.lastActivated);
    // lastActiveLabel->setStyleSheet("color: #999; font-size: 10px;");
    // lastActiveLabel->setAlignment(Qt::AlignRight);

    statusLayout->addWidget(m_statusLabel);
    // statusLayout->addWidget(lastActiveLabel);
    statusLayout->addStretch();

    mainLayout->addLayout(statusLayout);

    // Background style
    this->setStyleSheet("SceneItemWidget { background-color: white; border: 1px solid #ddd; border-radius: 5px; }"
                        "SceneItemWidget:hover { background-color: #f9f9f9; border: 1px solid #bbb; }");
    this->setAttribute(Qt::WA_StyledBackground, true);
}
