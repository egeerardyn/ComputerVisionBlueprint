#include "DivisionModel.h"

#include "Nodes/NodeHelpInfo.h"

namespace {
const NodeHelpRegistration kDivisionModelHelp(QStringLiteral("Division"),
                                              makeNodeHelp(QStringLiteral("Divides the first input by the second input and outputs the quotient."),
                                                           QStringLiteral("https://github.com/PabloPicose/ComputerVisionBlueprint")));
}
