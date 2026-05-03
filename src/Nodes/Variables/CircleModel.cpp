#include "CircleModel.h"

#include <QSignalBlocker>

#include "ui_CircleForm.h"

CircleModel::CircleModel() {
    updateOutputs({});
}

CircleModel::~CircleModel() {
}

QString CircleModel::caption() const {
    return "Circle";
}

QString CircleModel::name() const {
    return "Circle";
}

unsigned CircleModel::nPorts(const QtNodes::PortType portType) const {
    switch (portType) {
        case QtNodes::PortType::In:
            return 1;
        case QtNodes::PortType::Out:
            return 3;
        default:
            return 0;
    }
}

QtNodes::NodeDataType CircleModel::dataType(const QtNodes::PortType portType, const QtNodes::PortIndex portIndex) const {
    switch (portType) {
        case QtNodes::PortType::In:
            return CircleData().type();
        case QtNodes::PortType::Out:
            switch (portIndex) {
                case 0:
                    return CircleData().type();
                case 1:
                    return PointData().type();
                case 2:
                    return VariantData(0).type();
                default:
                    return VariantData().type();
            }
        default:
            return VariantData().type();
    }
}

void CircleModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, const QtNodes::PortIndex portIndex) {
    Q_UNUSED(portIndex)

    if (const auto circleData = std::dynamic_pointer_cast<CircleData>(nodeData)) {
        m_inCircleData = circleData;
        updateOutputs(circleData->circle());
        if (m_ui) {
            m_ui->sb_center_x->setEnabled(false);
            m_ui->sb_center_y->setEnabled(false);
            m_ui->sb_radius->setEnabled(false);
            applyCircleToUi(circleData->circle());
        }
    } else {
        m_inCircleData.reset();
        updateOutputs({});
        if (m_ui) {
            m_ui->sb_center_x->setEnabled(true);
            m_ui->sb_center_y->setEnabled(true);
            m_ui->sb_radius->setEnabled(true);
            applyCircleToUi(m_circle);
        }
    }
}

std::shared_ptr<QtNodes::NodeData> CircleModel::outData(const QtNodes::PortIndex port) {
    switch (port) {
        case 0:
            return m_outCircleData;
        case 1:
            return m_outCenterData;
        case 2:
            return m_outRadiusData;
        default:
            return std::make_shared<VariantData>();
    }
}

QWidget* CircleModel::embeddedWidget() {
    if (!m_widget) {
        m_ui = std::make_unique<Ui::CircleForm>();
        m_widget = new QWidget();
        m_ui->setupUi(m_widget);

        connect(m_ui->sb_center_x, QOverload<int>::of(&QSpinBox::valueChanged), this, &CircleModel::updateCircleFromUi);
        connect(m_ui->sb_center_y, QOverload<int>::of(&QSpinBox::valueChanged), this, &CircleModel::updateCircleFromUi);
        connect(m_ui->sb_radius, QOverload<int>::of(&QSpinBox::valueChanged), this, &CircleModel::updateCircleFromUi);

        applyCircleToUi(m_circle);
    }
    return m_widget;
}

bool CircleModel::portCaptionVisible(QtNodes::PortType, QtNodes::PortIndex) const {
    return true;
}

QString CircleModel::portCaption(const QtNodes::PortType port, const QtNodes::PortIndex portIndex) const {
    switch (port) {
        case QtNodes::PortType::In:
            return "Circle";
        case QtNodes::PortType::Out:
            switch (portIndex) {
                case 0:
                    return "Circle";
                case 1:
                    return "Center";
                case 2:
                    return "Radius";
                default:
                    return QString();
            }
        default:
            return NodeDelegateModel::portCaption(port, portIndex);
    }
}

void CircleModel::updateCircleFromUi() {
    if (!m_ui || !m_inCircleData.expired()) {
        return;
    }

    Circle circle;
    circle.center = QPoint(m_ui->sb_center_x->value(), m_ui->sb_center_y->value());
    circle.radius = m_ui->sb_radius->value();
    updateOutputs(circle);
}

void CircleModel::applyCircleToUi(const Circle& circle) const {
    if (!m_ui) {
        return;
    }

    QSignalBlocker blockCenterX(m_ui->sb_center_x);
    QSignalBlocker blockCenterY(m_ui->sb_center_y);
    QSignalBlocker blockRadius(m_ui->sb_radius);
    m_ui->sb_center_x->setValue(circle.center.x());
    m_ui->sb_center_y->setValue(circle.center.y());
    m_ui->sb_radius->setValue(circle.radius);
}

void CircleModel::updateOutputs(const Circle& circle) {
    m_circle = circle;
    m_outCircleData = std::make_shared<CircleData>(circle);
    m_outCenterData = std::make_shared<PointData>(circle.center);
    m_outRadiusData = std::make_shared<VariantData>(circle.radius);

    if (m_ui) {
        applyCircleToUi(circle);
    }

    emit dataUpdated(0);
    emit dataUpdated(1);
    emit dataUpdated(2);
}
