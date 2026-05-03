#pragma once

#include <QMetaType>
#include <QString>
#include <QUrl>

namespace QtNodes {
class NodeDelegateModel;
}

struct NodeHelpInfo {
    QString summary;
    QUrl onlineHelpUrl;

    bool hasSummary() const { return !summary.trimmed().isEmpty(); }
    bool hasOnlineHelp() const { return onlineHelpUrl.isValid() && !onlineHelpUrl.isEmpty(); }
};

class NodeHelpProvider {
public:
    virtual ~NodeHelpProvider() = default;

    virtual NodeHelpInfo nodeHelpInfo() const = 0;
};

class NodeHelpRegistration {
public:
    NodeHelpRegistration(QString modelName, NodeHelpInfo helpInfo);
};

NodeHelpInfo makeNodeHelp(QString summary, QString url = QString());

NodeHelpInfo nodeHelpInfoForModelName(const QString& modelName);

NodeHelpInfo nodeHelpInfoForModel(const QtNodes::NodeDelegateModel& model);
