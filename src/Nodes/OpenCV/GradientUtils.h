#ifndef GRADIENTUTILS_H
#define GRADIENTUTILS_H

#include <opencv2/core.hpp>

enum class GradientOutputMode {
    X = 0,
    Y = 1,
    Magnitude = 2
};

cv::Mat ComputeSobelGradient(const cv::Mat& source,
                             int ksize,
                             double scale,
                             double delta,
                             GradientOutputMode mode);

cv::Mat ComputeScharrGradient(const cv::Mat& source,
                              double scale,
                              double delta,
                              GradientOutputMode mode);

#endif // GRADIENTUTILS_H
