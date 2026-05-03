#include "TemplateMatchingUtils.h"

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

bool IsMinimumBestMethod(const int method) {
    return method == cv::TM_SQDIFF || method == cv::TM_SQDIFF_NORMED;
}
} // namespace

TemplateMatchResult MatchTemplateImage(const cv::Mat& image,
                                       const cv::Mat& templateImage,
                                       const int method) {
    const cv::Mat sourceGray = EnsureGray(image);
    const cv::Mat templateGray = EnsureGray(templateImage);
    if (sourceGray.empty() || templateGray.empty()) {
        return {};
    }

    if (templateGray.cols > sourceGray.cols || templateGray.rows > sourceGray.rows) {
        return {};
    }

    cv::Mat result;
    cv::matchTemplate(sourceGray, templateGray, result, method);

    double minValue = 0.0;
    double maxValue = 0.0;
    cv::Point minLocation;
    cv::Point maxLocation;
    cv::minMaxLoc(result, &minValue, &maxValue, &minLocation, &maxLocation);

    TemplateMatchResult match;
    match.templateSize = templateGray.size();
    if (IsMinimumBestMethod(method)) {
        match.location = minLocation;
        match.score = minValue;
    } else {
        match.location = maxLocation;
        match.score = maxValue;
    }
    match.valid = true;
    return match;
}

cv::Mat DrawTemplateMatch(const cv::Mat& image,
                          const TemplateMatchResult& match,
                          const cv::Scalar& color,
                          const int thickness) {
    if (image.empty()) {
        return {};
    }

    cv::Mat output;
    if (image.channels() == 1) {
        cv::cvtColor(image, output, cv::COLOR_GRAY2BGR);
    } else if (image.channels() == 4) {
        cv::cvtColor(image, output, cv::COLOR_BGRA2BGR);
    } else {
        output = image.clone();
    }

    if (!match.valid) {
        return output;
    }

    const cv::Rect roi(match.location, match.templateSize);
    cv::rectangle(output, roi, color, std::max(1, thickness), cv::LINE_AA);
    return output;
}
