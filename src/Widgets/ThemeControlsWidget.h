#ifndef THEMECONTROLSWIDGET_H
#define THEMECONTROLSWIDGET_H

#include <QColor>
#include <QWidget>

class QComboBox;
class QPushButton;
class QGroupBox;

class ThemeManager;

class ThemeControlsWidget final : public QWidget {
    Q_OBJECT

public:
    explicit ThemeControlsWidget(ThemeManager* themeManager, QWidget* parent = nullptr);

private slots:
    void chooseWindowColor();

    void chooseBaseColor();

    void chooseTextColor();

    void chooseAccentColor();

    void syncFromTheme();

private:
    void updateColorButton(QPushButton* button, const QColor& color) const;

private:
    ThemeManager* m_themeManager = nullptr;
    QComboBox* m_presetCombo = nullptr;
    QGroupBox* m_customColorsGroup = nullptr;
    QPushButton* m_windowColorButton = nullptr;
    QPushButton* m_baseColorButton = nullptr;
    QPushButton* m_textColorButton = nullptr;
    QPushButton* m_accentColorButton = nullptr;
};

#endif //THEMECONTROLSWIDGET_H
