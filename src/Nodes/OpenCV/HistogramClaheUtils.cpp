#include "HistogramClaheUtils.h"

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
} // namespace

cv::Mat ComputeGrayscaleHistogramPlot(const cv::Mat& source,
                                      int bins,
                                      const int width,
                                      const int height) {
    const cv::Mat gray = EnsureGray(source);
    if (gray.empty() || bins <= 0 || width <= 0 || height <= 0) {
        return {};
    }

    bins = std::min(256, std::max(8, bins));
    const float range[] = {0.0f, 256.0f};
    const float* ranges[] = {range};
    int channels[] = {0};

    cv::Mat hist;
    cv::calcHist(&gray, 1, channels, cv::Mat(), hist, 1, &bins, ranges, true, false);

    cv::Mat histNorm;
    cv::normalize(hist, histNorm, 0.0, static_cast<double>(height - 1), cv::NORM_MINMAX);

    cv::Mat plot(height, width, CV_8UC3, cv::Scalar(16, 16, 16));
    const int binWidth = std::max(1, cvRound(static_cast<double>(width) / bins));

    for (int i = 1; i < bins; ++i) {
        const cv::Point p1((i - 1) * binWidth, height - cvRound(histNorm.at<float>(i - 1)) - 1);
        const cv::Point p2(i * binWidth, height - cvRound(histNorm.at<float>(i)) - 1);
        cv::line(plot, p1, p2, cv::Scalar(80, 220, 80), 2, cv::LINE_AA);
    }

    return plot;
}

cv::Mat ApplyClaheGrayscale(const cv::Mat& source,
                            const double clipLimit,
                            int tileSize) {
    cv::Mat gray = EnsureGray(source);
    if (gray.empty()) {
        return {};
    }

    tileSize = std::max(2, tileSize);

    cv::Mat output;
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(std::max(0.01, clipLimit), cv::Size(tileSize, tileSize));
    clahe->apply(gray, output);
    return output;
}
