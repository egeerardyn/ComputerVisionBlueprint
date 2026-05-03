#ifndef CONVERTDEPTHCHANNELSMODEL_H
#define CONVERTDEPTHCHANNELSMODEL_H

#include <QtNodes/NodeDelegateModel>

#include "Nodes/Data/ImageData.h"
#include "Nodes/OpenCV/TypeConversionUtils.h"

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QSpinBox;
class QWidget;

class ConvertDepthChannelsModel final : public QtNodes::NodeDelegateModel {
    Q_OBJECT

public:
    ConvertDepthChannelsModel();

    ~ConvertDepthChannelsModel() override;

    QString caption() const override;

    QString name() const override;

    unsigned nPorts(QtNodes::PortType portType) const override;

    QtNodes::NodeDataType dataType(QtNodes::PortType portType, QtNodes::PortIndex portIndex) const override;

    void setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex portIndex) override;

    std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex port) override;

    QWidget* embeddedWidget() override;

private:
    void requestProcess();

    QImage getImageToProcess() const;

private:
    QWidget* m_widget = nullptr;
    QComboBox* m_depthCombo = nullptr;
    QComboBox* m_channelsCombo = nullptr;
    QDoubleSpinBox* m_alphaSpinBox = nullptr;
    QDoubleSpinBox* m_betaSpinBox = nullptr;
    QCheckBox* m_saturateCheckBox = nullptr;
    QSpinBox* m_timeSpinBox = nullptr;

    std::weak_ptr<ImageData> m_inImageData;
    std::shared_ptr<ImageData> m_outImageData;

    int m_targetDepth = CV_8U;
    ChannelConversionMode m_channelMode = ChannelConversionMode::Keep;
    double m_alpha = 1.0;
    double m_beta = 0.0;
    bool m_saturateIntegers = true;
};

#endif // CONVERTDEPTHCHANNELSMODEL_H
