#include <opencv2/imgproc.hpp>

#include "Nodes/OpenCV/HistogramClaheUtils.h"

namespace {
int run() {
    cv::Mat image(160, 240, CV_8UC1);
    for (int y = 0; y < image.rows; ++y) {
        for (int x = 0; x < image.cols; ++x) {
            image.at<uchar>(y, x) = static_cast<uchar>((x * 255) / std::max(1, image.cols - 1));
        }
    }

    const cv::Mat plot = ComputeGrayscaleHistogramPlot(image, 64, 320, 200);
    if (plot.empty() || plot.type() != CV_8UC3 || plot.cols != 320 || plot.rows != 200) {
        return 1;
    }

    cv::Mat lowContrast(120, 120, CV_8UC1, cv::Scalar(90));
    cv::rectangle(lowContrast, cv::Rect(30, 30, 60, 60), cv::Scalar(110), cv::FILLED);
    const cv::Mat clahe = ApplyClaheGrayscale(lowContrast, 2.0, 8);
    if (clahe.empty() || clahe.type() != CV_8UC1 || clahe.size() != lowContrast.size()) {
        return 2;
    }

    const double difference = cv::norm(clahe, lowContrast, cv::NORM_L1);
    if (difference <= 0.0) {
        return 3;
    }

    return 0;
}
} // namespace

int main() {
    return run();
}
