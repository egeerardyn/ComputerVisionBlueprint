#ifndef TEMPLATEMATCHINGUTILS_H
#define TEMPLATEMATCHINGUTILS_H

#include <opencv2/core.hpp>

struct TemplateMatchResult {
    cv::Point location;
    double score = 0.0;
    cv::Size templateSize;
    bool valid = false;
};

TemplateMatchResult MatchTemplateImage(const cv::Mat& image,
                                       const cv::Mat& templateImage,
                                       int method);

cv::Mat DrawTemplateMatch(const cv::Mat& image,
                          const TemplateMatchResult& match,
                          const cv::Scalar& color,
                          int thickness);

#endif // TEMPLATEMATCHINGUTILS_H
