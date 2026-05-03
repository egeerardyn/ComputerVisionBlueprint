#include "HelpWidget.h"

#include <QDesktopServices>
#include <QLabel>
#include <QPushButton>
#include <QTextBrowser>
#include <QVBoxLayout>

#include "Nodes/NodeHelpInfo.h"

namespace {
    QString nodeHelpToHtml(const QString& summary) {
        return QStringLiteral("<p>%1</p>").arg(summary.toHtmlEscaped().replace('\n', QStringLiteral("<br/>")));
    }
}

HelpWidget::HelpWidget(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(8);

    m_titleLabel = new QLabel(this);
    m_titleLabel->setWordWrap(true);
    layout->addWidget(m_titleLabel);

    m_browser = new QTextBrowser(this);
    m_browser->setOpenExternalLinks(false);
    layout->addWidget(m_browser, 1);

    m_onlineHelpButton = new QPushButton("Open online help", this);
    m_onlineHelpButton->hide();
    layout->addWidget(m_onlineHelpButton);

    connect(m_onlineHelpButton, &QPushButton::clicked, this, &HelpWidget::openCurrentOnlineHelp);

    showDefaultHelp();
}

void HelpWidget::showDefaultHelp() {
    m_titleLabel->setText("<b>Computer Vision Blueprint Help</b>");
    m_browser->setHtml(
        "<h3>Getting started</h3>"
        "<ul>"
        "<li>Drag nodes from the left panel into the scene.</li>"
        "<li>Connect compatible ports to build an image-processing pipeline.</li>"
        "<li>Select a node to see its brief description here.</li>"
        "<li>Use <b>File &gt; Save</b> and <b>File &gt; Load</b> to work with stored scenes.</li>"
        "</ul>"
        "<h3>Node help</h3>"
        "<ul>"
        "<li>When a single node is selected, this dock shows a short explanation of what it does.</li>"
        "<li>If a node has an online reference, use <b>Open online help</b> to launch it in your browser.</li>"
        "</ul>"
        "<h3>Themes</h3>"
        "<ul>"
        "<li>Use the <b>Theme</b> menu for Light, Dark, or Custom presets.</li>"
        "<li>Open <b>View &gt; Theme Controls</b> to tune custom colors.</li>"
        "<li>Theme choices persist automatically between runs.</li>"
        "</ul>"
        "<p>Tip: Keep this dock open while building workflows, or hide it from the <b>Help</b> menu.</p>");
    setOnlineHelpUrl(QUrl());
}

void HelpWidget::showNodeHelp(const QString& nodeTitle, const NodeHelpInfo& helpInfo) {
    m_titleLabel->setText(QStringLiteral("<b>%1</b>").arg(nodeTitle.toHtmlEscaped()));
    m_browser->setHtml(nodeHelpToHtml(helpInfo.hasSummary()
                                          ? helpInfo.summary
                                          : QStringLiteral("No node-specific help is available for this node yet.")));
    setOnlineHelpUrl(helpInfo.onlineHelpUrl);
}

void HelpWidget::showMultiSelectionHelp(const int selectedCount) {
    m_titleLabel->setText("<b>Multiple nodes selected</b>");
    m_browser->setHtml(QStringLiteral(
                           "<p>%1 nodes are selected. Select a single node to see its brief description and any available online help link.</p>")
                           .arg(selectedCount));
    setOnlineHelpUrl(QUrl());
}

void HelpWidget::openCurrentOnlineHelp() const {
    if (m_currentOnlineHelpUrl.isValid() && !m_currentOnlineHelpUrl.isEmpty()) {
        QDesktopServices::openUrl(m_currentOnlineHelpUrl);
    }
}

void HelpWidget::setOnlineHelpUrl(const QUrl& url) {
    m_currentOnlineHelpUrl = url;
    const bool hasOnlineHelp = m_currentOnlineHelpUrl.isValid() && !m_currentOnlineHelpUrl.isEmpty();
    m_onlineHelpButton->setVisible(hasOnlineHelp);
    m_onlineHelpButton->setEnabled(hasOnlineHelp);
}
