#ifndef MARKERSDATA_H
#define MARKERSDATA_H

#include <QtNodes/NodeData>

#include <QColor>
#include <QHash>
#include <QImage>

#include <opencv2/core.hpp>

class MarkersData final : public QtNodes::NodeData {
public:
    MarkersData() = default;

    explicit MarkersData(const cv::Mat& markers)
        : m_markers(copyMarkers(markers)),
          m_preview(createPreview(m_markers)) {
    }

    QtNodes::NodeDataType type() const override {
        return {"markers", "Markers"};
    }

    bool isNull() const {
        return m_markers.empty();
    }

    cv::Mat markers() const {
        return m_markers.clone();
    }

    QImage preview() const {
        return m_preview;
    }

    static QColor colorForLabel(const int label) {
        if (label == -1) {
            return QColor(255, 0, 0);
        }
        if (label <= 0) {
            return QColor(0, 0, 0);
        }

        const auto hash = static_cast<uint>(qHash(label));
        return QColor::fromHsv(static_cast<int>(hash % 360U),
                               160 + static_cast<int>(hash % 96U),
                               180 + static_cast<int>((hash / 7U) % 76U));
    }

private:
    static cv::Mat copyMarkers(const cv::Mat& markers) {
        if (markers.empty()) {
            return {};
        }

        if (markers.type() == CV_32S) {
            return markers.clone();
        }

        cv::Mat converted;
        markers.convertTo(converted, CV_32S);
        return converted;
    }

    static QImage createPreview(const cv::Mat& markers) {
        if (markers.empty()) {
            return {};
        }

        QImage preview(markers.cols, markers.rows, QImage::Format_RGB888);
        for (int y = 0; y < markers.rows; ++y) {
            const auto* row = markers.ptr<int>(y);
            auto* scanLine = preview.scanLine(y);
            for (int x = 0; x < markers.cols; ++x) {
                const QColor color = colorForLabel(row[x]);
                scanLine[(x * 3) + 0] = static_cast<uchar>(color.red());
                scanLine[(x * 3) + 1] = static_cast<uchar>(color.green());
                scanLine[(x * 3) + 2] = static_cast<uchar>(color.blue());
            }
        }

        return preview;
    }

private:
    cv::Mat m_markers;
    QImage m_preview;
};

#endif //MARKERSDATA_H
