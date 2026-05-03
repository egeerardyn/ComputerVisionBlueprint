#include "MorphologyExModel.h"
#include "Nodes/NodeHelpInfo.h"

namespace {
const NodeHelpRegistration kMorphologyExModelHelp(QStringLiteral("MorphologyEx"),
                                                  makeNodeHelp(QStringLiteral("Runs higher-level morphology operations such as opening, closing, gradient, top hat, or black hat."),
                                                               QStringLiteral("https://docs.opencv.org/4.x/d4/d86/group__imgproc__filter.html")));
}

MorphologyExModel::MorphologyExModel()
    : MorphologyModelBase("MorphologyEx", cv::MORPH_OPEN, {{"Open", cv::MORPH_OPEN},
                                                           {"Close", cv::MORPH_CLOSE},
                                                           {"Gradient", cv::MORPH_GRADIENT},
                                                           {"Top Hat", cv::MORPH_TOPHAT},
                                                           {"Black Hat", cv::MORPH_BLACKHAT},
                                                           {"Hit or Miss", cv::MORPH_HITMISS}}) {
}

MorphologyExModel::~MorphologyExModel() {
}
