#include "DilateModel.h"
#include "Nodes/NodeHelpInfo.h"

namespace {
const NodeHelpRegistration kDilateModelHelp(QStringLiteral("Dilate"),
                                            makeNodeHelp(QStringLiteral("Expands bright regions according to the selected structuring element. This is useful for filling gaps and strengthening blobs."),
                                                         QStringLiteral("https://docs.opencv.org/4.x/d4/d86/group__imgproc__filter.html")));
}

DilateModel::DilateModel()
    : MorphologyModelBase("Dilate", cv::MORPH_DILATE) {
}

DilateModel::~DilateModel() {
}
