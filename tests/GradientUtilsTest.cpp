#include <opencv2/imgproc.hpp>

#include "Nodes/OpenCV/GradientUtils.h"

namespace {
int run() {
    cv::Mat image(128, 128, CV_8UC1, cv::Scalar(0));
    cv::rectangle(image, cv::Rect(32, 32, 64, 64), cv::Scalar(255), cv::FILLED);

    const cv::Mat sobelMag = ComputeSobelGradient(image, 3, 1.0, 0.0, GradientOutputMode::Magnitude);
    if (sobelMag.empty() || sobelMag.type() != CV_32FC1) {
        return 1;
    }

    const double sobelSum = cv::sum(cv::abs(sobelMag))[0];
    if (sobelSum <= 0.0) {
        return 2;
    }

    cv::Mat color;
    cv::cvtColor(image, color, cv::COLOR_GRAY2BGR);
    const cv::Mat scharrMag = ComputeScharrGradient(color, 1.0, 0.0, GradientOutputMode::Magnitude);
    if (scharrMag.empty() || scharrMag.type() != CV_32FC1) {
        return 3;
    }

    const double scharrSum = cv::sum(cv::abs(scharrMag))[0];
    if (scharrSum <= 0.0) {
        return 4;
    }

    const cv::Mat sobelX = ComputeSobelGradient(image, 3, 1.0, 0.0, GradientOutputMode::X);
    if (sobelX.empty() || sobelX.type() != CV_32FC1) {
        return 5;
    }

    return 0;
}
} // namespace

int main() {
    return run();
}
