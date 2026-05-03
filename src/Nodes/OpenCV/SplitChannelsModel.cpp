#include "SplitChannelsModel.h"

#include "Nodes/NodeHelpInfo.h"
#include "Nodes/Conversor/MatQt.h"
#include "Nodes/OpenCV/OpenCVFilterUtils.h"

#include <algorithm>
#include <vector>

namespace {
const NodeHelpRegistration kSplitChannelsModelHelp(
    QStringLiteral("Split Channels"),
    makeNodeHelp(QStringLiteral("Splits the input image into independent B, G, R, and optional A single-channel outputs using cv::split."),
                 QStringLiteral("https://docs.opencv.org/4.x/d2/de8/group__core__array.html#ga8027f4e6fb8c947fbf3f4a4643d14d14")));
}

SplitChannelsModel::SplitChannelsModel() {
    clearOutputs();
}

SplitChannelsModel::~SplitChannelsModel() {
}

QString SplitChannelsModel::caption() const {
    return "Split Channels";
}

QString SplitChannelsModel::name() const {
    return "Split Channels";
}

unsigned SplitChannelsModel::nPorts(const QtNodes::PortType portType) const {
    switch (portType) {
        case QtNodes::PortType::In:
            return 1;
        case QtNodes::PortType::Out:
            return 4;
        default:
            return 0;
    }
}

QtNodes::NodeDataType SplitChannelsModel::dataType(const QtNodes::PortType portType,
                                                   const QtNodes::PortIndex portIndex) const {
    Q_UNUSED(portType)
    Q_UNUSED(portIndex)
    return ImageData().type();
}

void SplitChannelsModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, const QtNodes::PortIndex portIndex) {
    Q_UNUSED(portIndex)
    m_inImageData = std::dynamic_pointer_cast<ImageData>(nodeData);

    const auto lock = m_inImageData.lock();
    if (!lock) {
        clearOutputs();
        for (QtNodes::PortIndex index = 0; index < 4; ++index) {
            emit dataUpdated(index);
        }
        return;
    }

    const cv::Mat source = QImageToMat(lock->image());
    if (source.empty()) {
        clearOutputs();
        for (QtNodes::PortIndex index = 0; index < 4; ++index) {
            emit dataUpdated(index);
        }
        return;
    }

    std::vector<cv::Mat> channels;
    try {
        cv::split(source, channels);
    } catch (const cv::Exception& exception) {
        LogOpenCvError("SplitChannels", exception);
        clearOutputs();
        for (QtNodes::PortIndex index = 0; index < 4; ++index) {
            emit dataUpdated(index);
        }
        return;
    }

    clearOutputs();
    const int maxChannels = std::min<int>(static_cast<int>(channels.size()), 4);
    for (int channelIndex = 0; channelIndex < maxChannels; ++channelIndex) {
        m_outImageData[channelIndex] = std::make_shared<ImageData>(MatToQImage(channels[channelIndex]));
    }

    for (QtNodes::PortIndex index = 0; index < 4; ++index) {
        emit dataUpdated(index);
    }
}

std::shared_ptr<QtNodes::NodeData> SplitChannelsModel::outData(const QtNodes::PortIndex port) {
    if (port >= 0 && port < static_cast<QtNodes::PortIndex>(m_outImageData.size())) {
        return m_outImageData[port];
    }

    return nullptr;
}

QWidget* SplitChannelsModel::embeddedWidget() {
    return nullptr;
}

bool SplitChannelsModel::portCaptionVisible(const QtNodes::PortType portType, const QtNodes::PortIndex portIndex) const {
    Q_UNUSED(portType)
    Q_UNUSED(portIndex)
    return true;
}

QString SplitChannelsModel::portCaption(const QtNodes::PortType portType, const QtNodes::PortIndex portIndex) const {
    if (portType == QtNodes::PortType::In) {
        return QStringLiteral("Image");
    }

    switch (portIndex) {
        case 0:
            return QStringLiteral("B");
        case 1:
            return QStringLiteral("G");
        case 2:
            return QStringLiteral("R");
        case 3:
            return QStringLiteral("A");
        default:
            return QStringLiteral("Channel");
    }
}

void SplitChannelsModel::clearOutputs() {
    for (auto& out : m_outImageData) {
        out.reset();
    }
}
