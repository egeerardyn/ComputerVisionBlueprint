#ifndef HOUGHCIRCLESUTILS_H
#define HOUGHCIRCLESUTILS_H

#include <vector>

#include <opencv2/core.hpp>

std::vector<cv::Vec3f> DetectHoughCircles(const cv::Mat& source,
                                          double dp,
                                          double minDist,
                                          double param1,
                                          double param2,
                                          int minRadius,
                                          int maxRadius);

#endif // HOUGHCIRCLESUTILS_H
