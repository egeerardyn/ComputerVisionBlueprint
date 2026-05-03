#include "MorphologyExModel.h"

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
