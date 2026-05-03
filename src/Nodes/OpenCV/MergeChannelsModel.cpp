#include "MergeChannelsModel.h"

#include "Nodes/NodeHelpInfo.h"
#include "Nodes/Conversor/MatQt.h"
#include "Nodes/OpenCV/OpenCVFilterUtils.h"

#include <vector>

namespace {
const NodeHelpRegistration kMergeChannelsModelHelp(
    QStringLiteral("Merge Channels"),
    makeNodeHelp(QStringLiteral("Merges B, G, R, and optional A single-channel images into one multi-channel image using cv::merge."),
                 QStringLiteral("https://docs.opencv.org/4.x/d2/de8/group__core__array.html#ga8027f4e6fb8c947fbf3f4a4643d14d14")));
}

MergeChannelsModel::MergeChannelsModel() {
}

MergeChannelsModel::~MergeChannelsModel() {
}

QString MergeChannelsModel::caption() const {
    return "Merge Channels";
}

QString MergeChannelsModel::name() const {
    return "Merge Channels";
}

unsigned MergeChannelsModel::nPorts(const QtNodes::PortType portType) const {
    switch (portType) {
        case QtNodes::PortType::In:
            return 4;
        case QtNodes::PortType::Out:
            return 1;
        default:
            return 0;
    }
}

QtNodes::NodeDataType MergeChannelsModel::dataType(const QtNodes::PortType portType,
                                                   const QtNodes::PortIndex portIndex) const {
    Q_UNUSED(portType)
    Q_UNUSED(portIndex)
    return ImageData().type();
}

void MergeChannelsModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, const QtNodes::PortIndex portIndex) {
    if (portIndex < 0 || portIndex >= static_cast<QtNodes::PortIndex>(m_inImageData.size())) {
        return;
    }

    m_inImageData[portIndex] = std::dynamic_pointer_cast<ImageData>(nodeData);
    requestProcess();
}

std::shared_ptr<QtNodes::NodeData> MergeChannelsModel::outData(const QtNodes::PortIndex port) {
    Q_UNUSED(port)
    return m_outImageData;
}

QWidget* MergeChannelsModel::embeddedWidget() {
    return nullptr;
}

bool MergeChannelsModel::portCaptionVisible(const QtNodes::PortType portType, const QtNodes::PortIndex portIndex) const {
    Q_UNUSED(portType)
    Q_UNUSED(portIndex)
    return true;
}

QString MergeChannelsModel::portCaption(const QtNodes::PortType portType, const QtNodes::PortIndex portIndex) const {
    if (portType == QtNodes::PortType::Out) {
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
            return QStringLiteral("A [OPT]");
        default:
            return QStringLiteral("Channel");
    }
}

void MergeChannelsModel::requestProcess() {
    const auto blueLock = m_inImageData[0].lock();
    const auto greenLock = m_inImageData[1].lock();
    const auto redLock = m_inImageData[2].lock();

    if (!blueLock || !greenLock || !redLock) {
        m_outImageData.reset();
        emit dataUpdated(0);
        return;
    }

    const cv::Mat b = QImageToMat(blueLock->image());
    const cv::Mat g = QImageToMat(greenLock->image());
    const cv::Mat r = QImageToMat(redLock->image());

    if (b.empty() || g.empty() || r.empty()) {
        m_outImageData.reset();
        emit dataUpdated(0);
        return;
    }

    if (b.channels() != 1 || g.channels() != 1 || r.channels() != 1) {
        m_outImageData.reset();
        emit dataUpdated(0);
        return;
    }

    if (b.size() != g.size() || b.size() != r.size() || b.type() != g.type() || b.type() != r.type()) {
        m_outImageData.reset();
        emit dataUpdated(0);
        return;
    }

    std::vector<cv::Mat> channels = {b, g, r};

    if (const auto alphaLock = m_inImageData[3].lock()) {
        const cv::Mat a = QImageToMat(alphaLock->image());
        if (!a.empty() && a.channels() == 1 && a.size() == b.size() && a.type() == b.type()) {
            channels.push_back(a);
        }
    }

    cv::Mat merged;
    try {
        cv::merge(channels, merged);
    } catch (const cv::Exception& exception) {
        LogOpenCvError("MergeChannels", exception);
        m_outImageData.reset();
        emit dataUpdated(0);
        return;
    }

    if (merged.empty()) {
        m_outImageData.reset();
    } else {
        m_outImageData = std::make_shared<ImageData>(MatToQImage(merged));
    }

    emit dataUpdated(0);
}
