#include "HistogramModel.h"

#include "Nodes/NodeHelpInfo.h"
#include "Nodes/Conversor/MatQt.h"
#include "Nodes/OpenCV/HistogramClaheUtils.h"
#include "Nodes/OpenCV/OpenCVFilterUtils.h"

#include <QAbstractSpinBox>
#include <QElapsedTimer>
#include <QFormLayout>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QtConcurrent/QtConcurrent>

namespace {
const NodeHelpRegistration kHistogramModelHelp(QStringLiteral("Histogram"),
                                               makeNodeHelp(QStringLiteral("Builds a grayscale histogram image from the input frame for quick intensity distribution inspection."),
                                                            QStringLiteral("https://docs.opencv.org/4.x/d1/db7/tutorial_py_histogram_begins.html")));
} // namespace

HistogramModel::HistogramModel() {
    connect(&m_watcher, &QFutureWatcher<std::tuple<QImage, quint64>>::finished, this, &HistogramModel::processFinished);
}

HistogramModel::~HistogramModel() {
}

QString HistogramModel::caption() const {
    return "Histogram";
}

QString HistogramModel::name() const {
    return "Histogram";
}

unsigned HistogramModel::nPorts(const QtNodes::PortType portType) const {
    switch (portType) {
        case QtNodes::PortType::In:
            return 1;
        case QtNodes::PortType::Out:
            return 1;
        default:
            return 0;
    }
}

QtNodes::NodeDataType HistogramModel::dataType(const QtNodes::PortType portType,
                                               const QtNodes::PortIndex portIndex) const {
    Q_UNUSED(portType)
    Q_UNUSED(portIndex)
    return ImageData().type();
}

void HistogramModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, const QtNodes::PortIndex portIndex) {
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

std::shared_ptr<QtNodes::NodeData> HistogramModel::outData(const QtNodes::PortIndex port) {
    Q_UNUSED(port)
    return m_outImageData;
}

QWidget* HistogramModel::embeddedWidget() {
    if (!m_widget) {
        m_widget = new QWidget();
        auto* rootLayout = new QVBoxLayout(m_widget);
        rootLayout->setContentsMargins(6, 6, 6, 6);

        auto* formLayout = new QFormLayout();

        m_binsSpinBox = new QSpinBox(m_widget);
        m_binsSpinBox->setRange(8, 256);
        m_binsSpinBox->setValue(m_bins);
        formLayout->addRow("Bins", m_binsSpinBox);

        m_widthSpinBox = new QSpinBox(m_widget);
        m_widthSpinBox->setRange(64, 2048);
        m_widthSpinBox->setValue(m_width);
        formLayout->addRow("Width", m_widthSpinBox);

        m_heightSpinBox = new QSpinBox(m_widget);
        m_heightSpinBox->setRange(64, 2048);
        m_heightSpinBox->setValue(m_height);
        formLayout->addRow("Height", m_heightSpinBox);

        m_timeSpinBox = new QSpinBox(m_widget);
        m_timeSpinBox->setRange(0, 999999999);
        m_timeSpinBox->setReadOnly(true);
        m_timeSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
        formLayout->addRow("Time ms", m_timeSpinBox);

        rootLayout->addLayout(formLayout);

        connect(m_binsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
            m_bins = value;
            m_lastImageToProcess = getImageToProcess();
            requestProcess();
        });
        connect(m_widthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
            m_width = value;
            m_lastImageToProcess = getImageToProcess();
            requestProcess();
        });
        connect(m_heightSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
            m_height = value;
            m_lastImageToProcess = getImageToProcess();
            requestProcess();
        });
    }

    return m_widget;
}

void HistogramModel::processFinished() {
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

void HistogramModel::requestProcess() {
    if (m_watcher.isRunning()) {
        return;
    }

    if (m_lastImageToProcess.isNull()) {
        return;
    }

    const auto future = QtConcurrent::run(processImage, m_lastImageToProcess, m_bins, m_width, m_height);
    m_lastImageToProcess = QImage();
    m_watcher.setFuture(future);
}

std::tuple<QImage, quint64> HistogramModel::processImage(const QImage image,
                                                         const int bins,
                                                         const int width,
                                                         const int height) {
    QElapsedTimer timer;
    timer.start();

    const cv::Mat source = QImageToMat(image);
    if (source.empty()) {
        return {QImage(), timer.elapsed()};
    }

    try {
        const cv::Mat plot = ComputeGrayscaleHistogramPlot(source, bins, width, height);
        return {MatToQImage(plot), timer.elapsed()};
    } catch (const cv::Exception& exception) {
        LogOpenCvError("Histogram", exception);
    } catch (const std::exception& exception) {
        LogStdError("Histogram", exception);
    } catch (...) {
        LogUnknownError("Histogram");
    }

    return {QImage(), timer.elapsed()};
}

QImage HistogramModel::getImageToProcess() const {
    if (const auto lock = m_inImageData.lock()) {
        return lock->image();
    }

    return {};
}
