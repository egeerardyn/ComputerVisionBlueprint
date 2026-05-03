#include "ErodeModel.h"
#include "Nodes/NodeHelpInfo.h"

namespace {
const NodeHelpRegistration kErodeModelHelp(QStringLiteral("Erode"),
                                           makeNodeHelp(QStringLiteral("Shrinks bright regions according to the selected structuring element. This is useful for removing small foreground noise."),
                                                        QStringLiteral("https://docs.opencv.org/4.x/d4/d86/group__imgproc__filter.html")));
}

ErodeModel::ErodeModel()
    : MorphologyModelBase("Erode", cv::MORPH_ERODE) {
}

ErodeModel::~ErodeModel() {
}
