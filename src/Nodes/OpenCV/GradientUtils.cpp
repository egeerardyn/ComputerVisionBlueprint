#include "GradientUtils.h"

#include <opencv2/imgproc.hpp>

namespace {
cv::Mat EnsureGray(const cv::Mat& source) {
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

cv::Mat ComputeGradient(const cv::Mat& source,
                        const bool useScharr,
                        int ksize,
                        const double scale,
                        const double delta,
                        const GradientOutputMode mode) {
    const cv::Mat gray = EnsureGray(source);
    if (gray.empty()) {
        return {};
    }

    cv::Mat gradX;
    cv::Mat gradY;

    const int depth = CV_32F;
    if (useScharr) {
        cv::Scharr(gray, gradX, depth, 1, 0, scale, delta);
        cv::Scharr(gray, gradY, depth, 0, 1, scale, delta);
    } else {
        if (ksize % 2 == 0) {
            ++ksize;
        }
        ksize = std::max(1, ksize);
        cv::Sobel(gray, gradX, depth, 1, 0, ksize, scale, delta);
        cv::Sobel(gray, gradY, depth, 0, 1, ksize, scale, delta);
    }

    switch (mode) {
        case GradientOutputMode::X:
            return gradX;
        case GradientOutputMode::Y:
            return gradY;
        case GradientOutputMode::Magnitude: {
            cv::Mat magnitude;
            cv::magnitude(gradX, gradY, magnitude);
            return magnitude;
        }
        default:
            return {};
    }
}
} // namespace

cv::Mat ComputeSobelGradient(const cv::Mat& source,
                             const int ksize,
                             const double scale,
                             const double delta,
                             const GradientOutputMode mode) {
    return ComputeGradient(source, false, ksize, scale, delta, mode);
}

cv::Mat ComputeScharrGradient(const cv::Mat& source,
                              const double scale,
                              const double delta,
                              const GradientOutputMode mode) {
    return ComputeGradient(source, true, 3, scale, delta, mode);
}
