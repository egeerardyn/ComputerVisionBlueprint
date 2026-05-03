#include "HoughCirclesUtils.h"

#include <opencv2/imgproc.hpp>

namespace {
cv::Mat EnsureGray8(const cv::Mat& source) {
    if (source.empty()) {
        return {};
    }

    cv::Mat gray;
    switch (source.channels()) {
        case 1:
            gray = source;
            break;
        case 3:
            cv::cvtColor(source, gray, cv::COLOR_BGR2GRAY);
            break;
        case 4:
            cv::cvtColor(source, gray, cv::COLOR_BGRA2GRAY);
            break;
        default:
            return {};
    }

    if (gray.depth() != CV_8U) {
        cv::Mat normalized;
        cv::normalize(gray, normalized, 0, 255, cv::NORM_MINMAX);
        normalized.convertTo(gray, CV_8U);
    }

    return gray;
}
} // namespace

std::vector<cv::Vec3f> DetectHoughCircles(const cv::Mat& source,
                                          const double dp,
                                          const double minDist,
                                          const double param1,
                                          const double param2,
                                          const int minRadius,
                                          const int maxRadius) {
    const cv::Mat gray = EnsureGray8(source);
    if (gray.empty()) {
        return {};
    }

    cv::Mat blurred;
    cv::medianBlur(gray, blurred, 5);

    std::vector<cv::Vec3f> circles;
    cv::HoughCircles(blurred,
                     circles,
                     cv::HOUGH_GRADIENT,
                     dp,
                     minDist,
                     param1,
                     param2,
                     minRadius,
                     maxRadius);
    return circles;
}
