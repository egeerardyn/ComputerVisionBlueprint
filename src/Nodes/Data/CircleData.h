#ifndef CIRCLEDATA_H
#define CIRCLEDATA_H

#include <QtNodes/NodeData>
#include <QPoint>
#include <QtMath>

#include <opencv2/core/types.hpp>

struct Circle {
    QPoint center;
    int radius = 0;

    cv::Vec3f toCV() const {
        return {static_cast<float>(center.x()), static_cast<float>(center.y()), static_cast<float>(radius)};
    }
};

Q_DECLARE_METATYPE(Circle)

class CircleData final : public QtNodes::NodeData {
public:
    CircleData() = default;

    explicit CircleData(const Circle& circle) : m_circle(circle) {
    }

    explicit CircleData(const QPoint& center, const int radius) : m_circle({center, radius}) {
    }

    explicit CircleData(const cv::Vec3f& circle)
        : m_circle({QPoint(qRound(circle[0]), qRound(circle[1])), qRound(circle[2])}) {
    }

    Circle circle() const {
        return m_circle;
    }

    QtNodes::NodeDataType type() const override {
        return {"circle", "Circle"};
    }

private:
    Circle m_circle;
};

#endif //CIRCLEDATA_H
