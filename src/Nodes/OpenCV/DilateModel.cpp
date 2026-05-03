#include "DilateModel.h"

DilateModel::DilateModel()
    : MorphologyModelBase("Dilate", cv::MORPH_DILATE) {
}

DilateModel::~DilateModel() {
}
