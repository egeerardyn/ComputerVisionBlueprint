#include "ErodeModel.h"

ErodeModel::ErodeModel()
    : MorphologyModelBase("Erode", cv::MORPH_ERODE) {
}

ErodeModel::~ErodeModel() {
}
