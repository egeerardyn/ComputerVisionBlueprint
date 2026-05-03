#include "FromIndexedColorModel.h"

#include "Nodes/NodeHelpInfo.h"
#include "Nodes/Images/IndexedColorUtils.h"

namespace {
const NodeHelpRegistration kFromIndexedColorModelHelp(QStringLiteral("From Indexed Color"),
                                                      makeNodeHelp(QStringLiteral("Converts indexed color images back to regular RGB images for downstream processing."),
                                                                   QStringLiteral("https://doc.qt.io/qt-6/qimage.html#Format-enum")));
} // namespace

FromIndexedColorModel::FromIndexedColorModel() {
}

FromIndexedColorModel::~FromIndexedColorModel() {
}

QString FromIndexedColorModel::caption() const {
    return "From Indexed Color";
}

QString FromIndexedColorModel::name() const {
    return "From Indexed Color";
}

unsigned FromIndexedColorModel::nPorts(const QtNodes::PortType portType) const {
    switch (portType) {
        case QtNodes::PortType::In:
            return 1;
        case QtNodes::PortType::Out:
            return 1;
        default:
            return 0;
    }
}

QtNodes::NodeDataType FromIndexedColorModel::dataType(const QtNodes::PortType portType,
                                                      const QtNodes::PortIndex portIndex) const {
    Q_UNUSED(portType)
    Q_UNUSED(portIndex)
    return ImageData().type();
}

void FromIndexedColorModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, const QtNodes::PortIndex portIndex) {
    Q_UNUSED(portIndex)

    m_inImageData = std::dynamic_pointer_cast<ImageData>(nodeData);
    if (const auto lock = m_inImageData.lock()) {
        m_outImageData = std::make_shared<ImageData>(ConvertFromIndexedToRgb(lock->image()));
    } else {
        m_outImageData.reset();
    }

    emit dataUpdated(0);
}

std::shared_ptr<QtNodes::NodeData> FromIndexedColorModel::outData(const QtNodes::PortIndex port) {
    Q_UNUSED(port)
    return m_outImageData;
}

QWidget* FromIndexedColorModel::embeddedWidget() {
    return nullptr;
}
