#include "AdditionModel.h"

#include "Nodes/NodeHelpInfo.h"

namespace {
const NodeHelpRegistration kAdditionModelHelp(QStringLiteral("Addition"),
                                              makeNodeHelp(QStringLiteral("Adds two compatible values such as numbers or sizes and outputs the combined result."),
                                                           QStringLiteral("https://github.com/PabloPicose/ComputerVisionBlueprint")));
}
