#include "TypeConversionUtils.h"

#include <opencv2/imgproc.hpp>

namespace {
cv::Mat ConvertChannels(const cv::Mat& source, const ChannelConversionMode channelMode) {
    if (source.empty()) {
        return {};
    }

    const int sourceChannels = source.channels();
    const int targetChannels = [channelMode, sourceChannels]() {
        switch (channelMode) {
            case ChannelConversionMode::Keep:
                return sourceChannels;
            case ChannelConversionMode::Gray:
                return 1;
            case ChannelConversionMode::Bgr:
                return 3;
            case ChannelConversionMode::Bgra:
                return 4;
            default:
                return sourceChannels;
        }
    }();

    if (sourceChannels == targetChannels) {
        return source;
    }

    cv::Mat converted;
    if (sourceChannels == 1 && targetChannels == 3) {
        cv::cvtColor(source, converted, cv::COLOR_GRAY2BGR);
    } else if (sourceChannels == 1 && targetChannels == 4) {
        cv::cvtColor(source, converted, cv::COLOR_GRAY2BGRA);
    } else if (sourceChannels == 3 && targetChannels == 1) {
        cv::cvtColor(source, converted, cv::COLOR_BGR2GRAY);
    } else if (sourceChannels == 3 && targetChannels == 4) {
        cv::cvtColor(source, converted, cv::COLOR_BGR2BGRA);
    } else if (sourceChannels == 4 && targetChannels == 1) {
        cv::cvtColor(source, converted, cv::COLOR_BGRA2GRAY);
    } else if (sourceChannels == 4 && targetChannels == 3) {
        cv::cvtColor(source, converted, cv::COLOR_BGRA2BGR);
    }

    return converted;
}

void SaturateIntegerDepth(cv::Mat& mat) {
    if (mat.empty()) {
        return;
    }

    switch (mat.depth()) {
        case CV_8U:
            cv::min(mat, 255.0, mat);
            cv::max(mat, 0.0, mat);
            break;
        case CV_16U:
            cv::min(mat, 65535.0, mat);
            cv::max(mat, 0.0, mat);
            break;
        default:
            break;
    }
}
} // namespace

cv::Mat ConvertDepthAndChannels(const cv::Mat& source,
                                const int targetDepth,
                                const ChannelConversionMode channelMode,
                                const double alpha,
                                const double beta,
                                const bool saturateIntegers) {
    if (source.empty()) {
        return {};
    }

    cv::Mat channelConverted = ConvertChannels(source, channelMode);
    if (channelConverted.empty()) {
        return {};
    }

    cv::Mat converted;
    channelConverted.convertTo(converted, CV_MAKETYPE(targetDepth, channelConverted.channels()), alpha, beta);

    if (saturateIntegers) {
        SaturateIntegerDepth(converted);
    }

    return converted;
}
