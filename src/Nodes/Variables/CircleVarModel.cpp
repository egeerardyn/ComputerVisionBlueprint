#include "CircleVarModel.h"

#include "Nodes/NodeHelpInfo.h"

namespace {
const NodeHelpRegistration kCircleVarModelHelp(QStringLiteral("Circles"),
                                               makeNodeHelp(QStringLiteral("Collects one or more circle inputs into a circles output that can be reused by downstream drawing or analysis nodes."),
                                                            QStringLiteral("https://github.com/PabloPicose/ComputerVisionBlueprint")));
}

CircleVarModel::CircleVarModel() {
}

CircleVarModel::~CircleVarModel() {
}

QString CircleVarModel::caption() const {
    return "Circles";
}

QString CircleVarModel::name() const {
    return "Circles";
}

unsigned CircleVarModel::nPorts(const QtNodes::PortType portType) const {
    switch (portType) {
        case QtNodes::PortType::In:
            return 2;
        case QtNodes::PortType::Out:
            return m_outCircleList.size() + 1;
        default:
            return 0;
    }
}

QtNodes::NodeDataType CircleVarModel::dataType(const QtNodes::PortType portType, const QtNodes::PortIndex portIndex) const {
    switch (portType) {
        case QtNodes::PortType::In:
            return portIndex == 0 ? CirclesData().type() : CircleData().type();
        case QtNodes::PortType::Out:
            return portIndex == 0 ? CirclesData().type() : CircleData().type();
        default:
            return VariantData().type();
    }
}

void CircleVarModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, const QtNodes::PortIndex portIndex) {
    switch (portIndex) {
        case 0:
            for (auto it = m_inCirclesDataMap.begin(); it != m_inCirclesDataMap.end(); ++it) {
                if (it.value().expired()) {
                    m_inCirclesDataMap.insert(it.key(), std::dynamic_pointer_cast<CirclesData>(nodeData));
                    break;
                }
            }
            break;
        case 1:
            for (auto it = m_inCircleDataMap.begin(); it != m_inCircleDataMap.end(); ++it) {
                if (it.value().expired()) {
                    m_inCircleDataMap.insert(it.key(), std::dynamic_pointer_cast<CircleData>(nodeData));
                    break;
                }
            }
            break;
        default:
            break;
    }

    updateCircles();
}

std::shared_ptr<QtNodes::NodeData> CircleVarModel::outData(const QtNodes::PortIndex port) {
    if (port == 0) {
        return m_outCirclesData;
    }
    return port - 1 < m_outCircleList.size() ? m_outCircleList.at(port - 1) : nullptr;
}

QWidget* CircleVarModel::embeddedWidget() {
    return nullptr;
}

QString CircleVarModel::portCaption(const QtNodes::PortType port, const QtNodes::PortIndex portIndex) const {
    switch (port) {
        case QtNodes::PortType::In:
            return portIndex == 0 ? "Circles" : "Circle";
        case QtNodes::PortType::Out:
            return portIndex == 0 ? "Circles" : "Circle";
        default:
            return NodeDelegateModel::portCaption(port, portIndex);
    }
}

bool CircleVarModel::portCaptionVisible(QtNodes::PortType, QtNodes::PortIndex) const {
    return true;
}

QtNodes::ConnectionPolicy CircleVarModel::portConnectionPolicy(QtNodes::PortType, QtNodes::PortIndex) const {
    return QtNodes::ConnectionPolicy::Many;
}

void CircleVarModel::inputConnectionDeleted(QtNodes::ConnectionId const& connectionId) {
    if (connectionId.inPortIndex == 0) {
        m_inCirclesDataMap.remove(connectionId.outNodeId);
    } else if (connectionId.inPortIndex == 1) {
        m_inCircleDataMap.remove(connectionId.outNodeId);
    }
    updateCircles();
}

void CircleVarModel::inputConnectionCreated(QtNodes::ConnectionId const& connectionId) {
    if (connectionId.inPortIndex == 0) {
        m_inCirclesDataMap.insert(connectionId.outNodeId, std::weak_ptr<CirclesData>());
    } else if (connectionId.inPortIndex == 1) {
        m_inCircleDataMap.insert(connectionId.outNodeId, std::weak_ptr<CircleData>());
    }
}

void CircleVarModel::updateCircles() {
    Circles circles;

    for (auto it = m_inCirclesDataMap.begin(); it != m_inCirclesDataMap.end(); ++it) {
        if (const auto lock = it.value().lock()) {
            circles.append(lock->circles());
        }
    }

    for (auto it = m_inCircleDataMap.begin(); it != m_inCircleDataMap.end(); ++it) {
        if (const auto lock = it.value().lock()) {
            circles.append(lock->circle());
        }
    }

    m_outCircleList.clear();
    m_outCirclesData.reset();

    if (!circles.isEmpty()) {
        m_outCirclesData = std::make_shared<CirclesData>(circles);
        for (const Circle& circle : circles) {
            m_outCircleList.append(std::make_shared<CircleData>(circle));
        }
    }

    for (int i = 0; i < static_cast<int>(nPorts(QtNodes::PortType::Out)); ++i) {
        emit dataUpdated(i);
    }
}
