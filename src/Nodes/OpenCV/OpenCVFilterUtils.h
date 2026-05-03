#ifndef OPENCVFILTERUTILS_H
#define OPENCVFILTERUTILS_H

#include <QDebug>

#include <opencv2/opencv.hpp>

#include "Nodes/Conversor/MatQt.h"

inline int BorderTypeFromIndex(const int index) {
    switch (index) {
        case 0:
            return cv::BORDER_DEFAULT;
        case 1:
            return cv::BORDER_CONSTANT;
        case 2:
            return cv::BORDER_REPLICATE;
        case 3:
            return cv::BORDER_REFLECT;
        case 4:
            return cv::BORDER_REFLECT_101;
        default:
            return cv::BORDER_DEFAULT;
    }
}

inline int MorphShapeFromIndex(const int index) {
    switch (index) {
        case 0:
            return cv::MORPH_RECT;
        case 1:
            return cv::MORPH_CROSS;
        case 2:
            return cv::MORPH_ELLIPSE;
        default:
            return cv::MORPH_RECT;
    }
}

inline cv::Mat NormalizeMatForDisplay(const cv::Mat& mat) {
    if (mat.empty()) {
        return {};
    }

    if (mat.depth() == CV_8U) {
        return mat;
    }

    cv::Mat normalized;
    cv::normalize(mat, normalized, 0, 255, cv::NORM_MINMAX);

    cv::Mat converted;
    normalized.convertTo(converted, CV_MAKETYPE(CV_8U, normalized.channels()));
    return converted;
}

inline QImage MatToDisplayImage(const cv::Mat& mat) {
    return MatToQImage(NormalizeMatForDisplay(mat));
}

inline void LogOpenCvError(const char* nodeName, const cv::Exception& exception) {
    qWarning() << nodeName << "OpenCV error:" << exception.what();
}

inline void LogStdError(const char* nodeName, const std::exception& exception) {
    qWarning() << nodeName << "Error:" << exception.what();
}

inline void LogUnknownError(const char* nodeName) {
    qWarning() << nodeName << "Unknown error";
}

#endif //OPENCVFILTERUTILS_H
