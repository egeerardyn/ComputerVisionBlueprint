#include <opencv2/imgproc.hpp>

#include "Nodes/OpenCV/HoughCirclesUtils.h"

namespace {
int run() {
    cv::Mat image(220, 220, CV_8UC1, cv::Scalar(0));
    cv::circle(image, cv::Point(110, 110), 50, cv::Scalar(255), 3, cv::LINE_AA);

    const std::vector<cv::Vec3f> circles = DetectHoughCircles(image,
                                                               1.0,
                                                               20.0,
                                                               120.0,
                                                               20.0,
                                                               35,
                                                               65);

    if (circles.empty()) {
        return 1;
    }

    bool hasExpected = false;
    for (const cv::Vec3f& circle : circles) {
        const double dx = std::abs(circle[0] - 110.0f);
        const double dy = std::abs(circle[1] - 110.0f);
        const double dr = std::abs(circle[2] - 50.0f);
        if (dx <= 8.0 && dy <= 8.0 && dr <= 10.0) {
            hasExpected = true;
            break;
        }
    }

    if (!hasExpected) {
        return 2;
    }

    return 0;
}
} // namespace

int main() {
    return run();
}
