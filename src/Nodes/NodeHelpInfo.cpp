#include "NodeHelpInfo.h"

#include <QHash>

#include <QtNodes/NodeDelegateModel>

#include <utility>

namespace {
    QHash<QString, NodeHelpInfo>& nodeHelpRegistry() {
        static QHash<QString, NodeHelpInfo> registry;
        return registry;
    }
}

NodeHelpRegistration::NodeHelpRegistration(QString modelName, NodeHelpInfo helpInfo) {
    nodeHelpRegistry().insert(std::move(modelName), std::move(helpInfo));
}

NodeHelpInfo makeNodeHelp(QString summary, QString url) {
    return {std::move(summary), QUrl(std::move(url))};
}

NodeHelpInfo nodeHelpInfoForModelName(const QString& modelName) {
    const auto it = nodeHelpRegistry().constFind(modelName);
    if (it != nodeHelpRegistry().constEnd()) {
        return it.value();
    }

    return makeNodeHelp(QStringLiteral("No node-specific help is available for this node yet."),
                        QStringLiteral("https://github.com/PabloPicose/ComputerVisionBlueprint"));
}

NodeHelpInfo nodeHelpInfoForModel(const QtNodes::NodeDelegateModel& model) {
    if (const auto* provider = dynamic_cast<const NodeHelpProvider*>(&model)) {
        return provider->nodeHelpInfo();
    }

    return nodeHelpInfoForModelName(model.name());
}
