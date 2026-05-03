#ifndef CIRCLEVARMODEL_H
#define CIRCLEVARMODEL_H

#include <QMap>

#include <QtNodes/NodeDelegateModel>

#include "Nodes/Data/CircleData.h"
#include "Nodes/Data/CirclesData.h"
#include "Nodes/Data/VariantData.h"

class CircleVarModel final : public QtNodes::NodeDelegateModel {
    Q_OBJECT

public:
    CircleVarModel();

    ~CircleVarModel() override;

    QString caption() const override;

    QString name() const override;

    unsigned nPorts(QtNodes::PortType portType) const override;

    QtNodes::NodeDataType dataType(QtNodes::PortType portType, QtNodes::PortIndex portIndex) const override;

    void setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex portIndex) override;

    std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex port) override;

    QWidget* embeddedWidget() override;

    QString portCaption(QtNodes::PortType, QtNodes::PortIndex) const override;

    bool portCaptionVisible(QtNodes::PortType, QtNodes::PortIndex) const override;

    QtNodes::ConnectionPolicy portConnectionPolicy(QtNodes::PortType, QtNodes::PortIndex) const override;

private slots:
    void inputConnectionDeleted(QtNodes::ConnectionId const& connectionId) override;

    void inputConnectionCreated(QtNodes::ConnectionId const& connectionId) override;

private:
    void updateCircles();

private:
    QMap<int, std::weak_ptr<CirclesData>> m_inCirclesDataMap;
    QMap<int, std::weak_ptr<CircleData>> m_inCircleDataMap;

    std::shared_ptr<CirclesData> m_outCirclesData;
    QList<std::shared_ptr<CircleData>> m_outCircleList;
};

#endif //CIRCLEVARMODEL_H
