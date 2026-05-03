#ifndef MERGECHANNELSMODEL_H
#define MERGECHANNELSMODEL_H

#include <array>
#include <memory>

#include <QtNodes/NodeDelegateModel>

#include "Nodes/Data/ImageData.h"

class MergeChannelsModel final : public QtNodes::NodeDelegateModel {
    Q_OBJECT

public:
    MergeChannelsModel();

    ~MergeChannelsModel() override;

    QString caption() const override;

    QString name() const override;

    unsigned nPorts(QtNodes::PortType portType) const override;

    QtNodes::NodeDataType dataType(QtNodes::PortType portType, QtNodes::PortIndex portIndex) const override;

    void setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex portIndex) override;

    std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex port) override;

    QWidget* embeddedWidget() override;

    bool portCaptionVisible(QtNodes::PortType portType, QtNodes::PortIndex portIndex) const override;

    QString portCaption(QtNodes::PortType portType, QtNodes::PortIndex portIndex) const override;

private:
    void requestProcess();

private:
    std::array<std::weak_ptr<ImageData>, 4> m_inImageData;
    std::shared_ptr<ImageData> m_outImageData;
};

#endif // MERGECHANNELSMODEL_H
