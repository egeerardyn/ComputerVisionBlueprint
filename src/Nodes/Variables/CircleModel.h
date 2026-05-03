#ifndef CIRCLEMODEL_H
#define CIRCLEMODEL_H

#include <QtNodes/NodeDelegateModel>

#include "Nodes/Data/CircleData.h"
#include "Nodes/Data/PointData.h"
#include "Nodes/Data/VariantData.h"

namespace Ui {
    class CircleForm;
}

class CircleModel final : public QtNodes::NodeDelegateModel {
    Q_OBJECT

public:
    CircleModel();

    ~CircleModel() override;

    QString caption() const override;

    QString name() const override;

    unsigned nPorts(QtNodes::PortType portType) const override;

    QtNodes::NodeDataType dataType(QtNodes::PortType portType, QtNodes::PortIndex portIndex) const override;

    void setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex portIndex) override;

    std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex port) override;

    QWidget* embeddedWidget() override;

    bool portCaptionVisible(QtNodes::PortType, QtNodes::PortIndex) const override;

    QString portCaption(QtNodes::PortType, QtNodes::PortIndex) const override;

private slots:
    void updateCircleFromUi();

private:
    void applyCircleToUi(const Circle& circle) const;

    void updateOutputs(const Circle& circle);

private:
    QWidget* m_widget = nullptr;
    std::unique_ptr<Ui::CircleForm> m_ui;

    std::weak_ptr<CircleData> m_inCircleData;

    Circle m_circle;
    std::shared_ptr<CircleData> m_outCircleData;
    std::shared_ptr<PointData> m_outCenterData;
    std::shared_ptr<VariantData> m_outRadiusData;
};

#endif //CIRCLEMODEL_H
