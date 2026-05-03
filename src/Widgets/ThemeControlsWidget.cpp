#include "ThemeControlsWidget.h"

#include <QColorDialog>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSignalBlocker>
#include <QVBoxLayout>

#include "ThemeManager.h"

namespace {
    QColor buttonTextColor(const QColor& background) {
        return background.lightness() > 145 ? QColor(Qt::black) : QColor(Qt::white);
    }
}

ThemeControlsWidget::ThemeControlsWidget(ThemeManager* themeManager, QWidget* parent)
    : QWidget(parent), m_themeManager(themeManager) {
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(12, 12, 12, 12);
    rootLayout->setSpacing(12);

    auto* description = new QLabel(
        "Switch between built-in themes or fine tune a custom palette. Changes are saved automatically.",
        this);
    description->setWordWrap(true);
    rootLayout->addWidget(description);

    auto* presetLayout = new QFormLayout();
    m_presetCombo = new QComboBox(this);
    m_presetCombo->addItem(ThemeManager::presetLabel(ThemePreset::Dark));
    m_presetCombo->addItem(ThemeManager::presetLabel(ThemePreset::Light));
    m_presetCombo->addItem(ThemeManager::presetLabel(ThemePreset::Custom));
    presetLayout->addRow("Preset", m_presetCombo);
    rootLayout->addLayout(presetLayout);

    m_customColorsGroup = new QGroupBox("Custom colors", this);
    auto* colorsLayout = new QFormLayout(m_customColorsGroup);

    m_windowColorButton = new QPushButton(this);
    colorsLayout->addRow("Window", m_windowColorButton);
    m_baseColorButton = new QPushButton(this);
    colorsLayout->addRow("Canvas", m_baseColorButton);
    m_textColorButton = new QPushButton(this);
    colorsLayout->addRow("Text", m_textColorButton);
    m_accentColorButton = new QPushButton(this);
    colorsLayout->addRow("Accent", m_accentColorButton);

    auto* resetButton = new QPushButton("Reset custom colors", this);
    colorsLayout->addRow(QString(), resetButton);

    rootLayout->addWidget(m_customColorsGroup);
    rootLayout->addStretch(1);

    connect(m_presetCombo, &QComboBox::currentTextChanged, this, [this](const QString& text) {
        if (text == ThemeManager::presetLabel(ThemePreset::Light)) {
            m_themeManager->setPreset(ThemePreset::Light);
        } else if (text == ThemeManager::presetLabel(ThemePreset::Custom)) {
            m_themeManager->setPreset(ThemePreset::Custom);
        } else {
            m_themeManager->setPreset(ThemePreset::Dark);
        }
    });

    connect(m_windowColorButton, &QPushButton::clicked, this, &ThemeControlsWidget::chooseWindowColor);
    connect(m_baseColorButton, &QPushButton::clicked, this, &ThemeControlsWidget::chooseBaseColor);
    connect(m_textColorButton, &QPushButton::clicked, this, &ThemeControlsWidget::chooseTextColor);
    connect(m_accentColorButton, &QPushButton::clicked, this, &ThemeControlsWidget::chooseAccentColor);
    connect(resetButton, &QPushButton::clicked, m_themeManager, &ThemeManager::resetCustomColors);
    connect(m_themeManager, &ThemeManager::themeChanged, this, &ThemeControlsWidget::syncFromTheme);

    syncFromTheme();
}

void ThemeControlsWidget::chooseWindowColor() {
    const QColor color = QColorDialog::getColor(m_themeManager->configuration().windowColor, this, "Select window color");
    m_themeManager->setCustomWindowColor(color);
}

void ThemeControlsWidget::chooseBaseColor() {
    const QColor color = QColorDialog::getColor(m_themeManager->configuration().baseColor, this, "Select canvas color");
    m_themeManager->setCustomBaseColor(color);
}

void ThemeControlsWidget::chooseTextColor() {
    const QColor color = QColorDialog::getColor(m_themeManager->configuration().textColor, this, "Select text color");
    m_themeManager->setCustomTextColor(color);
}

void ThemeControlsWidget::chooseAccentColor() {
    const QColor color = QColorDialog::getColor(m_themeManager->configuration().accentColor, this, "Select accent color");
    m_themeManager->setCustomAccentColor(color);
}

void ThemeControlsWidget::syncFromTheme() {
    const ThemeConfiguration& configuration = m_themeManager->configuration();

    QSignalBlocker blocker(m_presetCombo);
    m_presetCombo->setCurrentText(ThemeManager::presetLabel(configuration.preset));

    m_customColorsGroup->setEnabled(configuration.preset == ThemePreset::Custom);
    updateColorButton(m_windowColorButton, configuration.windowColor);
    updateColorButton(m_baseColorButton, configuration.baseColor);
    updateColorButton(m_textColorButton, configuration.textColor);
    updateColorButton(m_accentColorButton, configuration.accentColor);
}

void ThemeControlsWidget::updateColorButton(QPushButton* button, const QColor& color) const {
    button->setText(color.name(QColor::HexRgb).toUpper());
    button->setStyleSheet(QString("background-color: %1; color: %2;")
        .arg(color.name(), buttonTextColor(color).name()));
}
