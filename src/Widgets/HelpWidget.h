#ifndef HELPWIDGET_H
#define HELPWIDGET_H

#include <QUrl>
#include <QWidget>

struct NodeHelpInfo;

class QLabel;
class QPushButton;
class QTextBrowser;

class HelpWidget final : public QWidget {
    Q_OBJECT

public:
    explicit HelpWidget(QWidget* parent = nullptr);

    void showDefaultHelp();

    void showNodeHelp(const QString& nodeTitle, const NodeHelpInfo& helpInfo);

    void showMultiSelectionHelp(int selectedCount);

private slots:
    void openCurrentOnlineHelp() const;

private:
    void setOnlineHelpUrl(const QUrl& url);

private:
    QLabel* m_titleLabel = nullptr;
    QTextBrowser* m_browser = nullptr;
    QPushButton* m_onlineHelpButton = nullptr;
    QUrl m_currentOnlineHelpUrl;
};

#endif //HELPWIDGET_H
