#ifndef TYPECONVERSIONUTILS_H
#define TYPECONVERSIONUTILS_H

#include <opencv2/core.hpp>

enum class ChannelConversionMode {
    Keep = 0,
    Gray = 1,
    Bgr = 2,
    Bgra = 3
};

cv::Mat ConvertDepthAndChannels(const cv::Mat& source,
                                int targetDepth,
                                ChannelConversionMode channelMode,
                                double alpha,
                                double beta,
                                bool saturateIntegers);

#endif // TYPECONVERSIONUTILS_H
