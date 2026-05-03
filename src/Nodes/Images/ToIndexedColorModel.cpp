#include "ToIndexedColorModel.h"

#include "Nodes/NodeHelpInfo.h"
#include "Nodes/Images/IndexedColorUtils.h"

namespace {
const NodeHelpRegistration kToIndexedColorModelHelp(QStringLiteral("To Indexed Color"),
                                                    makeNodeHelp(QStringLiteral("Converts a regular image into indexed color format (Indexed8 palette)."),
                                                                 QStringLiteral("https://doc.qt.io/qt-6/qimage.html#Format-enum")));
} // namespace

ToIndexedColorModel::ToIndexedColorModel() {
}

ToIndexedColorModel::~ToIndexedColorModel() {
}

QString ToIndexedColorModel::caption() const {
    return "To Indexed Color";
}

QString ToIndexedColorModel::name() const {
    return "To Indexed Color";
}

unsigned ToIndexedColorModel::nPorts(const QtNodes::PortType portType) const {
    switch (portType) {
        case QtNodes::PortType::In:
            return 1;
        case QtNodes::PortType::Out:
            return 1;
        default:
            return 0;
    }
}

QtNodes::NodeDataType ToIndexedColorModel::dataType(const QtNodes::PortType portType,
                                                    const QtNodes::PortIndex portIndex) const {
    Q_UNUSED(portType)
    Q_UNUSED(portIndex)
    return ImageData().type();
}

void ToIndexedColorModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, const QtNodes::PortIndex portIndex) {
    Q_UNUSED(portIndex)

    m_inImageData = std::dynamic_pointer_cast<ImageData>(nodeData);
    if (const auto lock = m_inImageData.lock()) {
        m_outImageData = std::make_shared<ImageData>(ConvertToIndexed8(lock->image()));
    } else {
        m_outImageData.reset();
    }

    emit dataUpdated(0);
}

std::shared_ptr<QtNodes::NodeData> ToIndexedColorModel::outData(const QtNodes::PortIndex port) {
    Q_UNUSED(port)
    return m_outImageData;
}

QWidget* ToIndexedColorModel::embeddedWidget() {
    return nullptr;
}
