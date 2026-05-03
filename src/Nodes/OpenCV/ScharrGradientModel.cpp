#include "ScharrGradientModel.h"

#include "Nodes/NodeHelpInfo.h"
#include "Nodes/Conversor/MatQt.h"
#include "Nodes/OpenCV/OpenCVFilterUtils.h"

#include <QAbstractSpinBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
#include <QFormLayout>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QtConcurrent/QtConcurrent>

namespace {
const NodeHelpRegistration kScharrGradientModelHelp(QStringLiteral("Scharr Gradient"),
                                                    makeNodeHelp(QStringLiteral("Computes Scharr x/y derivatives or gradient magnitude for stronger edge emphasis."),
                                                                 QStringLiteral("https://docs.opencv.org/4.x/d2/d2c/tutorial_sobel_derivatives.html")));

GradientOutputMode OutputModeFromIndex(const int index) {
    switch (index) {
        case 0:
            return GradientOutputMode::X;
        case 1:
            return GradientOutputMode::Y;
        case 2:
            return GradientOutputMode::Magnitude;
        default:
            return GradientOutputMode::Magnitude;
    }
}
} // namespace

ScharrGradientModel::ScharrGradientModel() {
    connect(&m_watcher, &QFutureWatcher<std::tuple<QImage, quint64>>::finished, this,
            &ScharrGradientModel::processFinished);
}

ScharrGradientModel::~ScharrGradientModel() {
}

QString ScharrGradientModel::caption() const {
    return "Scharr Gradient";
}

QString ScharrGradientModel::name() const {
    return "Scharr Gradient";
}

unsigned ScharrGradientModel::nPorts(const QtNodes::PortType portType) const {
    switch (portType) {
        case QtNodes::PortType::In:
            return 1;
        case QtNodes::PortType::Out:
            return 1;
        default:
            return 0;
    }
}

QtNodes::NodeDataType ScharrGradientModel::dataType(const QtNodes::PortType portType,
                                                    const QtNodes::PortIndex portIndex) const {
    Q_UNUSED(portType)
    Q_UNUSED(portIndex)
    return ImageData().type();
}

void ScharrGradientModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, const QtNodes::PortIndex portIndex) {
    Q_UNUSED(portIndex)
    m_inImageData = std::dynamic_pointer_cast<ImageData>(nodeData);
    if (const auto lock = m_inImageData.lock()) {
        m_lastImageToProcess = lock->image();
    } else {
        m_lastImageToProcess = QImage();
        m_outImageData.reset();
        emit dataUpdated(0);
    }
    requestProcess();
}

std::shared_ptr<QtNodes::NodeData> ScharrGradientModel::outData(const QtNodes::PortIndex port) {
    Q_UNUSED(port)
    return m_outImageData;
}

QWidget* ScharrGradientModel::embeddedWidget() {
    if (!m_widget) {
        m_widget = new QWidget();
        auto* rootLayout = new QVBoxLayout(m_widget);
        rootLayout->setContentsMargins(6, 6, 6, 6);

        auto* formLayout = new QFormLayout();

        m_modeCombo = new QComboBox(m_widget);
        m_modeCombo->addItems({"X", "Y", "Magnitude"});
        m_modeCombo->setCurrentIndex(2);
        formLayout->addRow("Output", m_modeCombo);

        m_scaleSpinBox = new QDoubleSpinBox(m_widget);
        m_scaleSpinBox->setRange(0.0, 64.0);
        m_scaleSpinBox->setDecimals(3);
        m_scaleSpinBox->setValue(m_scale);
        formLayout->addRow("Scale", m_scaleSpinBox);

        m_deltaSpinBox = new QDoubleSpinBox(m_widget);
        m_deltaSpinBox->setRange(-1024.0, 1024.0);
        m_deltaSpinBox->setDecimals(3);
        m_deltaSpinBox->setValue(m_delta);
        formLayout->addRow("Delta", m_deltaSpinBox);

        m_timeSpinBox = new QSpinBox(m_widget);
        m_timeSpinBox->setRange(0, 999999999);
        m_timeSpinBox->setReadOnly(true);
        m_timeSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
        formLayout->addRow("Time ms", m_timeSpinBox);

        rootLayout->addLayout(formLayout);

        connect(m_modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
            m_mode = OutputModeFromIndex(index);
            m_lastImageToProcess = getImageToProcess();
            requestProcess();
        });
        connect(m_scaleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
            m_scale = value;
            m_lastImageToProcess = getImageToProcess();
            requestProcess();
        });
        connect(m_deltaSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
            m_delta = value;
            m_lastImageToProcess = getImageToProcess();
            requestProcess();
        });
    }

    return m_widget;
}

void ScharrGradientModel::processFinished() {
    const auto [image, elapsed] = m_watcher.result();
    if (m_inImageData.expired()) {
        m_outImageData.reset();
    } else {
        m_outImageData = std::make_shared<ImageData>(image);
    }

    if (m_timeSpinBox) {
        m_timeSpinBox->setValue(static_cast<int>(elapsed));
    }

    emit dataUpdated(0);
    requestProcess();
}

void ScharrGradientModel::requestProcess() {
    if (m_watcher.isRunning()) {
        return;
    }

    if (m_lastImageToProcess.isNull()) {
        return;
    }

    const auto future = QtConcurrent::run(processImage, m_lastImageToProcess, m_scale, m_delta, m_mode);
    m_lastImageToProcess = QImage();
    m_watcher.setFuture(future);
}

std::tuple<QImage, quint64> ScharrGradientModel::processImage(const QImage image,
                                                              const double scale,
                                                              const double delta,
                                                              const GradientOutputMode mode) {
    QElapsedTimer timer;
    timer.start();

    const cv::Mat source = QImageToMat(image);
    if (source.empty()) {
        return {QImage(), timer.elapsed()};
    }

    try {
        const cv::Mat gradient = ComputeScharrGradient(source, scale, delta, mode);
        return {MatToDisplayImage(gradient), timer.elapsed()};
    } catch (const cv::Exception& exception) {
        LogOpenCvError("ScharrGradient", exception);
    } catch (const std::exception& exception) {
        LogStdError("ScharrGradient", exception);
    } catch (...) {
        LogUnknownError("ScharrGradient");
    }

    return {QImage(), timer.elapsed()};
}

QImage ScharrGradientModel::getImageToProcess() const {
    if (const auto lock = m_inImageData.lock()) {
        return lock->image();
    }

    return {};
}
