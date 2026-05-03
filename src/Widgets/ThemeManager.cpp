#include "ThemeManager.h"

#include <QApplication>
#include <QPalette>
#include <QSettings>
#include <QStyle>
#include <QtNodes/internal/ConnectionStyle.hpp>

namespace {
    ThemeConfiguration defaultDarkConfigurationInternal() {
        return {
            ThemePreset::Dark,
            QColor(53, 53, 53),
            QColor(35, 35, 35),
            QColor(240, 240, 240),
            QColor(42, 130, 218)
        };
    }

    QString toConnectionStyleJson(const QColor& normalColor, const QColor& accentColor) {
        return QString(R"({
  "ConnectionStyle": {
    "ConstructionColor": "%1",
    "NormalColor": "%2",
    "SelectedColor": "%3",
    "SelectedHaloColor": "%4",
    "HoveredColor": "%5",
    "LineWidth": 3.0,
    "ConstructionLineWidth": 2.0,
    "PointDiameter": 10.0,
    "UseDataDefinedColors": true
  }
})")
            .arg(accentColor.darker(115).name(),
                 normalColor.name(),
                 accentColor.name(),
                 accentColor.lighter(130).name(),
                 accentColor.lighter(120).name());
    }

    bool isDarkColor(const QColor& color) {
        return color.lightness() < 128;
    }

    QColor disabledText(const QColor& textColor, const QColor& windowColor) {
        if (isDarkColor(windowColor)) {
            return textColor.darker(160);
        }
        return textColor.darker(120);
    }

    QColor derivedAlternateBase(const QColor& baseColor, const QColor& windowColor) {
        if (isDarkColor(windowColor)) {
            return baseColor.lighter(112);
        }
        return baseColor.darker(104);
    }

    QColor derivedButton(const QColor& windowColor, const QColor& baseColor) {
        if (isDarkColor(windowColor)) {
            return windowColor.lighter(112);
        }
        return baseColor.darker(102);
    }

    ThemeConfiguration defaultLightConfiguration() {
        return {
            ThemePreset::Light,
            QColor(246, 246, 246),
            QColor(255, 255, 255),
            QColor(25, 25, 25),
            QColor(42, 130, 218)
        };
    }

    QPalette paletteForConfiguration(const ThemeConfiguration& configuration) {
        const ThemeConfiguration light = defaultLightConfiguration();
        const ThemeConfiguration dark = defaultDarkConfigurationInternal();
        const ThemeConfiguration active = configuration.preset == ThemePreset::Light ? light :
                                          configuration.preset == ThemePreset::Dark ? dark :
                                          configuration;

        const QColor windowColor = active.windowColor;
        const QColor baseColor = active.baseColor;
        const QColor textColor = active.textColor;
        const QColor accentColor = active.accentColor;
        const QColor buttonColor = derivedButton(windowColor, baseColor);
        const QColor alternateBase = derivedAlternateBase(baseColor, windowColor);
        const QColor disabled = disabledText(textColor, windowColor);
        const QColor highlightedText = accentColor.lightness() > 145 ? QColor(Qt::black) : QColor(Qt::white);

        QPalette palette;
        palette.setColor(QPalette::Window, windowColor);
        palette.setColor(QPalette::WindowText, textColor);
        palette.setColor(QPalette::Base, baseColor);
        palette.setColor(QPalette::AlternateBase, alternateBase);
        palette.setColor(QPalette::ToolTipBase, baseColor);
        palette.setColor(QPalette::ToolTipText, textColor);
        palette.setColor(QPalette::Text, textColor);
        palette.setColor(QPalette::Button, buttonColor);
        palette.setColor(QPalette::ButtonText, textColor);
        palette.setColor(QPalette::BrightText, QColor(Qt::red));
        palette.setColor(QPalette::Link, accentColor);
        palette.setColor(QPalette::Highlight, accentColor);
        palette.setColor(QPalette::HighlightedText, highlightedText);
        palette.setColor(QPalette::PlaceholderText, disabled);

        palette.setColor(QPalette::Disabled, QPalette::WindowText, disabled);
        palette.setColor(QPalette::Disabled, QPalette::Text, disabled);
        palette.setColor(QPalette::Disabled, QPalette::ButtonText, disabled);
        palette.setColor(QPalette::Disabled, QPalette::Highlight, accentColor.darker(140));
        palette.setColor(QPalette::Disabled, QPalette::HighlightedText, highlightedText);

        return palette;
    }
}

ThemeManager::ThemeManager(QObject* parent) : QObject(parent), m_configuration(defaultDarkConfigurationInternal()) {
    load();
}

const ThemeConfiguration& ThemeManager::configuration() const {
    return m_configuration;
}

void ThemeManager::apply() const {
    const QPalette palette = paletteForConfiguration(m_configuration);
    QApplication::setPalette(palette);

    const QColor tooltipBorder = palette.color(QPalette::Highlight);
    const QColor tooltipText = palette.color(QPalette::ToolTipText);
    const QColor tooltipBase = palette.color(QPalette::ToolTipBase);
    qApp->setStyleSheet(QString(
        "QToolTip {"
        " color: %1;"
        " background-color: %2;"
        " border: 1px solid %3;"
        " }"
        "QDockWidget::title { padding-left: 6px; }")
        .arg(tooltipText.name(), tooltipBase.name(), tooltipBorder.name()));

    const QColor normalColor = palette.color(QPalette::WindowText);
    const QColor accentColor = palette.color(QPalette::Highlight);
    QtNodes::ConnectionStyle::setConnectionStyle(toConnectionStyleJson(normalColor, accentColor));
}

QString ThemeManager::presetLabel(const ThemePreset preset) {
    switch (preset) {
        case ThemePreset::Light:
            return "Light";
        case ThemePreset::Dark:
            return "Dark";
        case ThemePreset::Custom:
            return "Custom";
        default:
            return "Custom";
    }
}

void ThemeManager::setPreset(const ThemePreset preset) {
    if (m_configuration.preset == preset) {
        return;
    }
    m_configuration.preset = preset;
    save();
    emit themeChanged();
}

void ThemeManager::setCustomWindowColor(const QColor& color) {
    setCustomColor(&ThemeConfiguration::windowColor, color);
}

void ThemeManager::setCustomBaseColor(const QColor& color) {
    setCustomColor(&ThemeConfiguration::baseColor, color);
}

void ThemeManager::setCustomTextColor(const QColor& color) {
    setCustomColor(&ThemeConfiguration::textColor, color);
}

void ThemeManager::setCustomAccentColor(const QColor& color) {
    setCustomColor(&ThemeConfiguration::accentColor, color);
}

void ThemeManager::resetCustomColors() {
    const ThemeConfiguration defaults = defaultDarkConfiguration();
    const bool changed = m_configuration.windowColor != defaults.windowColor ||
                         m_configuration.baseColor != defaults.baseColor ||
                         m_configuration.textColor != defaults.textColor ||
                         m_configuration.accentColor != defaults.accentColor ||
                         m_configuration.preset != ThemePreset::Custom;
    m_configuration.windowColor = defaults.windowColor;
    m_configuration.baseColor = defaults.baseColor;
    m_configuration.textColor = defaults.textColor;
    m_configuration.accentColor = defaults.accentColor;
    m_configuration.preset = ThemePreset::Custom;
    if (!changed) {
        return;
    }
    save();
    emit themeChanged();
}

void ThemeManager::load() {
    QSettings settings;
    settings.beginGroup("Theme");

    const ThemeConfiguration defaults = defaultDarkConfiguration();
    m_configuration.windowColor = settings.value("windowColor", defaults.windowColor).value<QColor>();
    m_configuration.baseColor = settings.value("baseColor", defaults.baseColor).value<QColor>();
    m_configuration.textColor = settings.value("textColor", defaults.textColor).value<QColor>();
    m_configuration.accentColor = settings.value("accentColor", defaults.accentColor).value<QColor>();

    const QString preset = settings.value("preset", presetLabel(defaults.preset)).toString();
    if (preset == presetLabel(ThemePreset::Light)) {
        m_configuration.preset = ThemePreset::Light;
    } else if (preset == presetLabel(ThemePreset::Custom)) {
        m_configuration.preset = ThemePreset::Custom;
    } else {
        m_configuration.preset = ThemePreset::Dark;
    }

    settings.endGroup();
}

void ThemeManager::save() const {
    QSettings settings;
    settings.beginGroup("Theme");
    settings.setValue("preset", presetLabel(m_configuration.preset));
    settings.setValue("windowColor", m_configuration.windowColor);
    settings.setValue("baseColor", m_configuration.baseColor);
    settings.setValue("textColor", m_configuration.textColor);
    settings.setValue("accentColor", m_configuration.accentColor);
    settings.endGroup();
}

void ThemeManager::setCustomColor(QColor ThemeConfiguration::*member, const QColor& color) {
    if (!color.isValid()) {
        return;
    }
    if (m_configuration.*member == color && m_configuration.preset == ThemePreset::Custom) {
        return;
    }
    m_configuration.*member = color;
    m_configuration.preset = ThemePreset::Custom;
    save();
    emit themeChanged();
}

ThemeConfiguration ThemeManager::defaultDarkConfiguration() {
    return defaultDarkConfigurationInternal();
}
