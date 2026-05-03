#include "SobelGradientModel.h"

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
const NodeHelpRegistration kSobelGradientModelHelp(QStringLiteral("Sobel Gradient"),
                                                   makeNodeHelp(QStringLiteral("Computes Sobel x/y derivatives or gradient magnitude for edge analysis."),
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

SobelGradientModel::SobelGradientModel() {
    connect(&m_watcher, &QFutureWatcher<std::tuple<QImage, quint64>>::finished, this, &SobelGradientModel::processFinished);
}

SobelGradientModel::~SobelGradientModel() {
}

QString SobelGradientModel::caption() const {
    return "Sobel Gradient";
}

QString SobelGradientModel::name() const {
    return "Sobel Gradient";
}

unsigned SobelGradientModel::nPorts(const QtNodes::PortType portType) const {
    switch (portType) {
        case QtNodes::PortType::In:
            return 1;
        case QtNodes::PortType::Out:
            return 1;
        default:
            return 0;
    }
}

QtNodes::NodeDataType SobelGradientModel::dataType(const QtNodes::PortType portType,
                                                   const QtNodes::PortIndex portIndex) const {
    Q_UNUSED(portType)
    Q_UNUSED(portIndex)
    return ImageData().type();
}

void SobelGradientModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, const QtNodes::PortIndex portIndex) {
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

std::shared_ptr<QtNodes::NodeData> SobelGradientModel::outData(const QtNodes::PortIndex port) {
    Q_UNUSED(port)
    return m_outImageData;
}

QWidget* SobelGradientModel::embeddedWidget() {
    if (!m_widget) {
        m_widget = new QWidget();
        auto* rootLayout = new QVBoxLayout(m_widget);
        rootLayout->setContentsMargins(6, 6, 6, 6);

        auto* formLayout = new QFormLayout();

        m_modeCombo = new QComboBox(m_widget);
        m_modeCombo->addItems({"X", "Y", "Magnitude"});
        m_modeCombo->setCurrentIndex(2);
        formLayout->addRow("Output", m_modeCombo);

        m_ksizeSpinBox = new QSpinBox(m_widget);
        m_ksizeSpinBox->setRange(1, 31);
        m_ksizeSpinBox->setSingleStep(2);
        m_ksizeSpinBox->setValue(m_ksize);
        formLayout->addRow("Kernel", m_ksizeSpinBox);

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
        connect(m_ksizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
            m_ksize = value;
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

void SobelGradientModel::processFinished() {
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

void SobelGradientModel::requestProcess() {
    if (m_watcher.isRunning()) {
        return;
    }

    if (m_lastImageToProcess.isNull()) {
        return;
    }

    const auto future = QtConcurrent::run(processImage, m_lastImageToProcess, m_ksize, m_scale, m_delta, m_mode);
    m_lastImageToProcess = QImage();
    m_watcher.setFuture(future);
}

std::tuple<QImage, quint64> SobelGradientModel::processImage(const QImage image,
                                                             const int ksize,
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
        const cv::Mat gradient = ComputeSobelGradient(source, ksize, scale, delta, mode);
        return {MatToDisplayImage(gradient), timer.elapsed()};
    } catch (const cv::Exception& exception) {
        LogOpenCvError("SobelGradient", exception);
    } catch (const std::exception& exception) {
        LogStdError("SobelGradient", exception);
    } catch (...) {
        LogUnknownError("SobelGradient");
    }

    return {QImage(), timer.elapsed()};
}

QImage SobelGradientModel::getImageToProcess() const {
    if (const auto lock = m_inImageData.lock()) {
        return lock->image();
    }

    return {};
}
