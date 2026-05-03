#include "ClaheModel.h"

#include "Nodes/NodeHelpInfo.h"
#include "Nodes/Conversor/MatQt.h"
#include "Nodes/OpenCV/HistogramClaheUtils.h"
#include "Nodes/OpenCV/OpenCVFilterUtils.h"

#include <QAbstractSpinBox>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
#include <QFormLayout>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QtConcurrent/QtConcurrent>

namespace {
const NodeHelpRegistration kClaheModelHelp(QStringLiteral("CLAHE"),
                                           makeNodeHelp(QStringLiteral("Applies contrast-limited adaptive histogram equalization to improve local contrast while reducing over-amplification."),
                                                        QStringLiteral("https://docs.opencv.org/4.x/d5/daf/tutorial_py_histogram_equalization.html")));
} // namespace

ClaheModel::ClaheModel() {
    connect(&m_watcher, &QFutureWatcher<std::tuple<QImage, quint64>>::finished, this, &ClaheModel::processFinished);
}

ClaheModel::~ClaheModel() {
}

QString ClaheModel::caption() const {
    return "CLAHE";
}

QString ClaheModel::name() const {
    return "CLAHE";
}

unsigned ClaheModel::nPorts(const QtNodes::PortType portType) const {
    switch (portType) {
        case QtNodes::PortType::In:
            return 1;
        case QtNodes::PortType::Out:
            return 1;
        default:
            return 0;
    }
}

QtNodes::NodeDataType ClaheModel::dataType(const QtNodes::PortType portType,
                                           const QtNodes::PortIndex portIndex) const {
    Q_UNUSED(portType)
    Q_UNUSED(portIndex)
    return ImageData().type();
}

void ClaheModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, const QtNodes::PortIndex portIndex) {
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

std::shared_ptr<QtNodes::NodeData> ClaheModel::outData(const QtNodes::PortIndex port) {
    Q_UNUSED(port)
    return m_outImageData;
}

QWidget* ClaheModel::embeddedWidget() {
    if (!m_widget) {
        m_widget = new QWidget();
        auto* rootLayout = new QVBoxLayout(m_widget);
        rootLayout->setContentsMargins(6, 6, 6, 6);

        auto* formLayout = new QFormLayout();

        m_clipLimitSpinBox = new QDoubleSpinBox(m_widget);
        m_clipLimitSpinBox->setRange(0.1, 40.0);
        m_clipLimitSpinBox->setDecimals(2);
        m_clipLimitSpinBox->setValue(m_clipLimit);
        formLayout->addRow("Clip limit", m_clipLimitSpinBox);

        m_tileSizeSpinBox = new QSpinBox(m_widget);
        m_tileSizeSpinBox->setRange(2, 64);
        m_tileSizeSpinBox->setValue(m_tileSize);
        formLayout->addRow("Tile size", m_tileSizeSpinBox);

        m_timeSpinBox = new QSpinBox(m_widget);
        m_timeSpinBox->setRange(0, 999999999);
        m_timeSpinBox->setReadOnly(true);
        m_timeSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
        formLayout->addRow("Time ms", m_timeSpinBox);

        rootLayout->addLayout(formLayout);

        connect(m_clipLimitSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
            m_clipLimit = value;
            m_lastImageToProcess = getImageToProcess();
            requestProcess();
        });
        connect(m_tileSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
            m_tileSize = value;
            m_lastImageToProcess = getImageToProcess();
            requestProcess();
        });
    }

    return m_widget;
}

void ClaheModel::processFinished() {
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

void ClaheModel::requestProcess() {
    if (m_watcher.isRunning()) {
        return;
    }

    if (m_lastImageToProcess.isNull()) {
        return;
    }

    const auto future = QtConcurrent::run(processImage, m_lastImageToProcess, m_clipLimit, m_tileSize);
    m_lastImageToProcess = QImage();
    m_watcher.setFuture(future);
}

std::tuple<QImage, quint64> ClaheModel::processImage(const QImage image,
                                                     const double clipLimit,
                                                     const int tileSize) {
    QElapsedTimer timer;
    timer.start();

    const cv::Mat source = QImageToMat(image);
    if (source.empty()) {
        return {QImage(), timer.elapsed()};
    }

    try {
        const cv::Mat clahe = ApplyClaheGrayscale(source, clipLimit, tileSize);
        return {MatToQImage(clahe), timer.elapsed()};
    } catch (const cv::Exception& exception) {
        LogOpenCvError("CLAHE", exception);
    } catch (const std::exception& exception) {
        LogStdError("CLAHE", exception);
    } catch (...) {
        LogUnknownError("CLAHE");
    }

    return {QImage(), timer.elapsed()};
}

QImage ClaheModel::getImageToProcess() const {
    if (const auto lock = m_inImageData.lock()) {
        return lock->image();
    }

    return {};
}
