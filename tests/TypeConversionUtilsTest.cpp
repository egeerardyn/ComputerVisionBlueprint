#include <opencv2/core.hpp>

#include "Nodes/OpenCV/TypeConversionUtils.h"

namespace {
int run() {
    cv::Mat sourceBgr(4, 6, CV_8UC3, cv::Scalar(10, 20, 30));

    const cv::Mat gray = ConvertDepthAndChannels(sourceBgr, CV_8U, ChannelConversionMode::Gray, 1.0, 0.0, true);
    if (gray.empty() || gray.type() != CV_8UC1 || gray.rows != sourceBgr.rows || gray.cols != sourceBgr.cols) {
        return 1;
    }

    const cv::Mat bgr16 = ConvertDepthAndChannels(sourceBgr, CV_16U, ChannelConversionMode::Keep, 4.0, 0.0, true);
    if (bgr16.empty() || bgr16.type() != CV_16UC3) {
        return 2;
    }

    cv::Mat sourceGray(2, 3, CV_8UC1, cv::Scalar(90));
    const cv::Mat bgra32f = ConvertDepthAndChannels(sourceGray, CV_32F, ChannelConversionMode::Bgra, 0.5, 1.0, false);
    if (bgra32f.empty() || bgra32f.type() != CV_32FC4) {
        return 3;
    }

    const cv::Mat emptyResult = ConvertDepthAndChannels(cv::Mat(), CV_8U, ChannelConversionMode::Keep, 1.0, 0.0, true);
    if (!emptyResult.empty()) {
        return 4;
    }

    return 0;
}
} // namespace

int main() {
    return run();
}
