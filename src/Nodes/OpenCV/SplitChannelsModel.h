#ifndef SPLITCHANNELSMODEL_H
#define SPLITCHANNELSMODEL_H

#include <array>
#include <memory>

#include <QtNodes/NodeDelegateModel>

#include "Nodes/Data/ImageData.h"

class SplitChannelsModel final : public QtNodes::NodeDelegateModel {
    Q_OBJECT

public:
    SplitChannelsModel();

    ~SplitChannelsModel() override;

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
    void clearOutputs();

private:
    std::weak_ptr<ImageData> m_inImageData;
    std::array<std::shared_ptr<ImageData>, 4> m_outImageData;
};

#endif // SPLITCHANNELSMODEL_H
