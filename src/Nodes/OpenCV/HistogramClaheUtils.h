#ifndef HISTOGRAMCLAHEUTILS_H
#define HISTOGRAMCLAHEUTILS_H

#include <opencv2/core.hpp>

cv::Mat ComputeGrayscaleHistogramPlot(const cv::Mat& source,
                                      int bins,
                                      int width,
                                      int height);

cv::Mat ApplyClaheGrayscale(const cv::Mat& source,
                            double clipLimit,
                            int tileSize);

#endif // HISTOGRAMCLAHEUTILS_H
