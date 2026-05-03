#ifndef CIRCLESDATA_H
#define CIRCLESDATA_H

#include <QtNodes/NodeData>
#include <QList>

#include "CircleData.h"

using Circles = QList<Circle>;

Q_DECLARE_METATYPE(Circles)

class CirclesData final : public QtNodes::NodeData {
public:
    CirclesData() = default;

    explicit CirclesData(const Circle& circle) : m_circles({circle}) {
    }

    explicit CirclesData(const Circles& circles) : m_circles(circles) {
    }

    explicit CirclesData(const std::vector<cv::Vec3f>& circles) {
        for (const cv::Vec3f& circle : circles) {
            m_circles.append(CircleData(circle).circle());
        }
    }

    Circles circles() const {
        return m_circles;
    }

    std::vector<cv::Vec3f> circlesCV() const {
        std::vector<cv::Vec3f> circles;
        circles.reserve(m_circles.size());
        for (const Circle& circle : m_circles) {
            circles.push_back(circle.toCV());
        }
        return circles;
    }

    QtNodes::NodeDataType type() const override {
        return {"circles", "Circles"};
    }

private:
    Circles m_circles;
};

#endif //CIRCLESDATA_H
