#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QColor>

enum class ThemePreset {
    Light,
    Dark,
    Custom
};

struct ThemeConfiguration {
    ThemePreset preset = ThemePreset::Dark;
    QColor windowColor;
    QColor baseColor;
    QColor textColor;
    QColor accentColor;
};

class ThemeManager final : public QObject {
    Q_OBJECT

public:
    explicit ThemeManager(QObject* parent = nullptr);

    const ThemeConfiguration& configuration() const;

    void apply() const;

    static QString presetLabel(ThemePreset preset);

public slots:
    void setPreset(ThemePreset preset);

    void setCustomWindowColor(const QColor& color);

    void setCustomBaseColor(const QColor& color);

    void setCustomTextColor(const QColor& color);

    void setCustomAccentColor(const QColor& color);

    void resetCustomColors();

signals:
    void themeChanged();

private:
    void load();

    void save() const;

    void setCustomColor(QColor ThemeConfiguration::*member, const QColor& color);

    static ThemeConfiguration defaultDarkConfiguration();

    ThemeConfiguration m_configuration;
};

#endif //THEMEMANAGER_H
