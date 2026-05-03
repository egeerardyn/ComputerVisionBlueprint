#include <opencv2/imgproc.hpp>

#include "Nodes/OpenCV/TemplateMatchingUtils.h"

namespace {
int run() {
    cv::Mat image(120, 160, CV_8UC1, cv::Scalar(0));
    const cv::Rect targetRoi(48, 36, 24, 24);
    image(targetRoi).setTo(cv::Scalar(40));
    cv::line(image, cv::Point(targetRoi.x + 3, targetRoi.y + 5),
             cv::Point(targetRoi.x + 20, targetRoi.y + 5), cv::Scalar(180), 2, cv::LINE_AA);
    cv::line(image, cv::Point(targetRoi.x + 7, targetRoi.y + 18),
             cv::Point(targetRoi.x + 18, targetRoi.y + 10), cv::Scalar(240), 2, cv::LINE_AA);
    cv::circle(image, cv::Point(targetRoi.x + 12, targetRoi.y + 12), 4, cv::Scalar(100), cv::FILLED, cv::LINE_AA);

    cv::Mat templ = image(targetRoi).clone();

    const TemplateMatchResult match = MatchTemplateImage(image, templ, cv::TM_CCOEFF_NORMED);
    if (!match.valid) {
        return 1;
    }

    if (std::abs(match.location.x - targetRoi.x) > 2 || std::abs(match.location.y - targetRoi.y) > 2) {
        return 2;
    }

    cv::Mat drawn = DrawTemplateMatch(image, match, cv::Scalar(255, 255, 255), 2);
    if (drawn.empty() || drawn.channels() != 3 || drawn.cols != image.cols || drawn.rows != image.rows) {
        return 3;
    }

    return 0;
}
} // namespace

int main() {
    return run();
}
