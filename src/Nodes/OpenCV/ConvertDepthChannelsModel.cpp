#include "ConvertDepthChannelsModel.h"

#include "Nodes/NodeHelpInfo.h"
#include "Nodes/Conversor/MatQt.h"
#include "Nodes/OpenCV/OpenCVFilterUtils.h"

#include <QCheckBox>
#include <QAbstractSpinBox>
#include <QElapsedTimer>
#include <QFormLayout>
#include <QLabel>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QVBoxLayout>
#include <QComboBox>

namespace {
const NodeHelpRegistration kConvertDepthChannelsModelHelp(
    QStringLiteral("Convert Depth/Channels"),
    makeNodeHelp(QStringLiteral("Converts image channel layout and numeric depth using OpenCV convertTo with configurable scale, shift, and integer saturation."),
                 QStringLiteral("https://docs.opencv.org/4.x/d2/de8/group__core__array.html#ga393164aa54bb9169ce0a8cc44e08ff22")));

int DepthFromIndex(const int index) {
    switch (index) {
        case 0:
            return CV_8U;
        case 1:
            return CV_16U;
        case 2:
            return CV_32F;
        default:
            return CV_8U;
    }
}

ChannelConversionMode ChannelsFromIndex(const int index) {
    switch (index) {
        case 0:
            return ChannelConversionMode::Keep;
        case 1:
            return ChannelConversionMode::Gray;
        case 2:
            return ChannelConversionMode::Bgr;
        case 3:
            return ChannelConversionMode::Bgra;
        default:
            return ChannelConversionMode::Keep;
    }
}
} // namespace

ConvertDepthChannelsModel::ConvertDepthChannelsModel() {
}

ConvertDepthChannelsModel::~ConvertDepthChannelsModel() {
}

QString ConvertDepthChannelsModel::caption() const {
    return "Convert Depth/Channels";
}

QString ConvertDepthChannelsModel::name() const {
    return "Convert Depth/Channels";
}

unsigned ConvertDepthChannelsModel::nPorts(const QtNodes::PortType portType) const {
    switch (portType) {
        case QtNodes::PortType::In:
            return 1;
        case QtNodes::PortType::Out:
            return 1;
        default:
            return 0;
    }
}

QtNodes::NodeDataType ConvertDepthChannelsModel::dataType(const QtNodes::PortType portType,
                                                          const QtNodes::PortIndex portIndex) const {
    Q_UNUSED(portType)
    Q_UNUSED(portIndex)
    return ImageData().type();
}

void ConvertDepthChannelsModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, const QtNodes::PortIndex portIndex) {
    Q_UNUSED(portIndex)
    m_inImageData = std::dynamic_pointer_cast<ImageData>(nodeData);
    requestProcess();
}

std::shared_ptr<QtNodes::NodeData> ConvertDepthChannelsModel::outData(const QtNodes::PortIndex port) {
    Q_UNUSED(port)
    return m_outImageData;
}

QWidget* ConvertDepthChannelsModel::embeddedWidget() {
    if (!m_widget) {
        m_widget = new QWidget();
        auto* rootLayout = new QVBoxLayout(m_widget);
        rootLayout->setContentsMargins(6, 6, 6, 6);

        auto* formLayout = new QFormLayout();

        m_depthCombo = new QComboBox(m_widget);
        m_depthCombo->addItems({"8U", "16U", "32F"});
        formLayout->addRow("Depth", m_depthCombo);

        m_channelsCombo = new QComboBox(m_widget);
        m_channelsCombo->addItems({"Keep", "Gray", "BGR", "BGRA"});
        formLayout->addRow("Channels", m_channelsCombo);

        m_alphaSpinBox = new QDoubleSpinBox(m_widget);
        m_alphaSpinBox->setRange(0.0, 1024.0);
        m_alphaSpinBox->setDecimals(4);
        m_alphaSpinBox->setSingleStep(0.1);
        m_alphaSpinBox->setValue(m_alpha);
        formLayout->addRow("Scale (alpha)", m_alphaSpinBox);

        m_betaSpinBox = new QDoubleSpinBox(m_widget);
        m_betaSpinBox->setRange(-2048.0, 2048.0);
        m_betaSpinBox->setDecimals(4);
        m_betaSpinBox->setSingleStep(1.0);
        m_betaSpinBox->setValue(m_beta);
        formLayout->addRow("Shift (beta)", m_betaSpinBox);

        m_saturateCheckBox = new QCheckBox("Clamp integer outputs", m_widget);
        m_saturateCheckBox->setChecked(m_saturateIntegers);
        formLayout->addRow(m_saturateCheckBox);

        m_timeSpinBox = new QSpinBox(m_widget);
        m_timeSpinBox->setRange(0, 999999999);
        m_timeSpinBox->setReadOnly(true);
        m_timeSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
        formLayout->addRow("Time ms", m_timeSpinBox);

        rootLayout->addLayout(formLayout);

        connect(m_depthCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
            m_targetDepth = DepthFromIndex(index);
            requestProcess();
        });
        connect(m_channelsCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
            m_channelMode = ChannelsFromIndex(index);
            requestProcess();
        });
        connect(m_alphaSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
            m_alpha = value;
            requestProcess();
        });
        connect(m_betaSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
            m_beta = value;
            requestProcess();
        });
        connect(m_saturateCheckBox, &QCheckBox::toggled, this, [this](bool checked) {
            m_saturateIntegers = checked;
            requestProcess();
        });
    }

    return m_widget;
}

void ConvertDepthChannelsModel::requestProcess() {
    const QImage image = getImageToProcess();
    if (image.isNull()) {
        m_outImageData.reset();
        emit dataUpdated(0);
        return;
    }

    QElapsedTimer timer;
    timer.start();

    const cv::Mat source = QImageToMat(image);
    cv::Mat converted;

    try {
        converted = ConvertDepthAndChannels(source, m_targetDepth, m_channelMode, m_alpha, m_beta, m_saturateIntegers);
    } catch (const cv::Exception& exception) {
        LogOpenCvError("ConvertDepthChannels", exception);
    } catch (const std::exception& exception) {
        LogStdError("ConvertDepthChannels", exception);
    } catch (...) {
        LogUnknownError("ConvertDepthChannels");
    }

    if (converted.empty()) {
        m_outImageData.reset();
    } else {
        m_outImageData = std::make_shared<ImageData>(MatToDisplayImage(converted));
    }

    if (m_timeSpinBox) {
        m_timeSpinBox->setValue(static_cast<int>(timer.elapsed()));
    }

    emit dataUpdated(0);
}

QImage ConvertDepthChannelsModel::getImageToProcess() const {
    if (const auto lock = m_inImageData.lock()) {
        return lock->image();
    }

    return {};
}
