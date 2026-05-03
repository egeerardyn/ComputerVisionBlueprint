#include "SubtractionModel.h"

#include "Nodes/NodeHelpInfo.h"

namespace {
const NodeHelpRegistration kSubtractionModelHelp(QStringLiteral("Subtraction"),
                                                 makeNodeHelp(QStringLiteral("Subtracts the second input from the first input and outputs the difference."),
                                                              QStringLiteral("https://github.com/PabloPicose/ComputerVisionBlueprint")));
}
