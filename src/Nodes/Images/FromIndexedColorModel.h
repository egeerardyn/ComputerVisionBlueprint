#ifndef FROMINDEXEDCOLORMODEL_H
#define FROMINDEXEDCOLORMODEL_H

#include <QtNodes/NodeDelegateModel>

#include "Nodes/Data/ImageData.h"

class FromIndexedColorModel final : public QtNodes::NodeDelegateModel {
    Q_OBJECT

public:
    FromIndexedColorModel();

    ~FromIndexedColorModel() override;

    QString caption() const override;

    QString name() const override;

    unsigned nPorts(QtNodes::PortType portType) const override;

    QtNodes::NodeDataType dataType(QtNodes::PortType portType, QtNodes::PortIndex portIndex) const override;

    void setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex portIndex) override;

    std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex port) override;

    QWidget* embeddedWidget() override;

private:
    std::weak_ptr<ImageData> m_inImageData;
    std::shared_ptr<ImageData> m_outImageData;
};

#endif // FROMINDEXEDCOLORMODEL_H
