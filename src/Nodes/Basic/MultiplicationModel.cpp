#include "MultiplicationModel.hpp"

#include "Nodes/NodeHelpInfo.h"

namespace {
const NodeHelpRegistration kMultiplicationModelHelp(QStringLiteral("Multiplication"),
                                                    makeNodeHelp(QStringLiteral("Multiplies two compatible values and outputs the product. Use it for scaling factors and simple calculations."),
                                                                 QStringLiteral("https://github.com/PabloPicose/ComputerVisionBlueprint")));
}
