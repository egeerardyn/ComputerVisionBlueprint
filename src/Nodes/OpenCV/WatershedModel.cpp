#include "WatershedModel.h"
#include "Nodes/NodeHelpInfo.h"

#include <algorithm>

#include <QAbstractSpinBox>
#include <QDebug>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
#include <QFormLayout>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QtConcurrent/QtConcurrent>

#include "Nodes/Conversor/MatQt.h"

namespace {
const NodeHelpRegistration kWatershedModelHelp(QStringLiteral("Watershed"),
                                               makeNodeHelp(QStringLiteral("Segments an image using marker-based watershed. Provide a source image and prepared markers to split touching regions."),
                                                            QStringLiteral("https://docs.opencv.org/4.x/d3/db4/tutorial_py_watershed.html")));

cv::Mat EnsureBgr(const cv::Mat& src) {
    if (src.channels() == 3) {
        return src;
    }

    cv::Mat bgr;
    if (src.channels() == 4) {
        cv::cvtColor(src, bgr, cv::COLOR_BGRA2BGR);
    } else {
        cv::cvtColor(src, bgr, cv::COLOR_GRAY2BGR);
    }
    return bgr;
}

int CountRegions(const cv::Mat& markers) {
    int maxLabel = 0;
    for (int y = 0; y < markers.rows; ++y) {
        const auto* row = markers.ptr<int>(y);
        for (int x = 0; x < markers.cols; ++x) {
            if (row[x] > maxLabel) {
                maxLabel = row[x];
            }
        }
    }

    return std::max(0, maxLabel - 1);
}

QImage CreateOverlay(const cv::Mat& sourceBgr, const cv::Mat& markers, const double overlayAlpha) {
    cv::Mat overlay = sourceBgr.clone();

    for (int y = 0; y < markers.rows; ++y) {
        const auto* markerRow = markers.ptr<int>(y);
        auto* overlayRow = overlay.ptr<cv::Vec3b>(y);
        for (int x = 0; x < markers.cols; ++x) {
            const int label = markerRow[x];
            cv::Vec3b& pixel = overlayRow[x];

            if (label == -1) {
                pixel = cv::Vec3b(0, 0, 255);
                continue;
            }

            if (label <= 1) {
                continue;
            }

            const QColor color = MarkersData::colorForLabel(label);
            pixel[0] = static_cast<uchar>((pixel[0] * (1.0 - overlayAlpha)) + (color.blue() * overlayAlpha));
            pixel[1] = static_cast<uchar>((pixel[1] * (1.0 - overlayAlpha)) + (color.green() * overlayAlpha));
            pixel[2] = static_cast<uchar>((pixel[2] * (1.0 - overlayAlpha)) + (color.red() * overlayAlpha));
        }
    }

    return MatToQImage(overlay);
}
} // namespace

WatershedModel::WatershedModel() {
    connect(&m_watcher, &QFutureWatcher<std::tuple<QImage, cv::Mat, int, quint64>>::finished, this,
            &WatershedModel::processFinished);
}

WatershedModel::~WatershedModel() {
}

QString WatershedModel::caption() const {
    return "Watershed";
}

QString WatershedModel::name() const {
    return "Watershed";
}

unsigned WatershedModel::nPorts(const QtNodes::PortType portType) const {
    switch (portType) {
        case QtNodes::PortType::In:
            return 2;
        case QtNodes::PortType::Out:
            return 3;
        default:
            return 0;
    }
}

QtNodes::NodeDataType WatershedModel::dataType(const QtNodes::PortType portType,
                                               const QtNodes::PortIndex portIndex) const {
    switch (portType) {
        case QtNodes::PortType::In:
            return portIndex == 0 ? ImageData().type() : MarkersData().type();
        case QtNodes::PortType::Out:
            switch (portIndex) {
                case 0:
                case 2:
                    return ImageData().type();
                case 1:
                    return MarkersData().type();
                default:
                    return ImageData().type();
            }
        default:
            return ImageData().type();
    }
}

void WatershedModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, const QtNodes::PortIndex portIndex) {
    switch (portIndex) {
        case 0:
            m_inImageData = std::dynamic_pointer_cast<ImageData>(nodeData);
            break;
        case 1:
            m_inMarkersData = std::dynamic_pointer_cast<MarkersData>(nodeData);
            break;
        default:
            break;
    }

    updatePendingInputs();
}

std::shared_ptr<QtNodes::NodeData> WatershedModel::outData(QtNodes::PortIndex port) {
    switch (port) {
        case 0:
            return m_outOverlayData;
        case 1:
            return m_outLabelsData;
        case 2:
            return m_outLabelsPreviewData;
        default:
            return nullptr;
    }
}

QWidget* WatershedModel::embeddedWidget() {
    if (!m_widget) {
        m_widget = new QWidget();
        auto* rootLayout = new QVBoxLayout(m_widget);
        rootLayout->setContentsMargins(6, 6, 6, 6);

        auto* formLayout = new QFormLayout();

        m_overlayAlphaSpinBox = new QDoubleSpinBox(m_widget);
        m_overlayAlphaSpinBox->setRange(0.0, 1.0);
        m_overlayAlphaSpinBox->setSingleStep(0.05);
        m_overlayAlphaSpinBox->setDecimals(2);
        m_overlayAlphaSpinBox->setValue(m_overlayAlpha);
        formLayout->addRow("Overlay alpha", m_overlayAlphaSpinBox);

        m_regionsSpinBox = new QSpinBox(m_widget);
        m_regionsSpinBox->setRange(0, 999999999);
        m_regionsSpinBox->setReadOnly(true);
        m_regionsSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
        formLayout->addRow("Regions", m_regionsSpinBox);

        m_timeSpinBox = new QDoubleSpinBox(m_widget);
        m_timeSpinBox->setRange(0.0, 999999999.0);
        m_timeSpinBox->setReadOnly(true);
        m_timeSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
        formLayout->addRow("Time ms", m_timeSpinBox);

        rootLayout->addLayout(formLayout);

        connect(m_overlayAlphaSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
                [this](double value) {
                    m_overlayAlpha = value;
                    updatePendingInputs();
                });
    }

    return m_widget;
}

bool WatershedModel::portCaptionVisible(QtNodes::PortType, QtNodes::PortIndex) const {
    return true;
}

QString WatershedModel::portCaption(const QtNodes::PortType portType, const QtNodes::PortIndex portIndex) const {
    switch (portType) {
        case QtNodes::PortType::In:
            return portIndex == 0 ? "Image" : "Markers";
        case QtNodes::PortType::Out:
            switch (portIndex) {
                case 0:
                    return "Overlay";
                case 1:
                    return "Labels";
                case 2:
                    return "Label Preview";
                default:
                    return {};
            }
        default:
            return NodeDelegateModel::portCaption(portType, portIndex);
    }
}

void WatershedModel::processFinished() {
    const auto [overlay, labels, regionCount, elapsed] = m_watcher.result();
    if (m_timeSpinBox) {
        m_timeSpinBox->setValue(static_cast<double>(elapsed));
    }
    if (m_regionsSpinBox) {
        m_regionsSpinBox->setValue(regionCount);
    }

    if (m_inImageData.expired() || m_inMarkersData.expired()) {
        m_outOverlayData.reset();
        m_outLabelsData.reset();
        m_outLabelsPreviewData.reset();
    } else {
        m_outOverlayData = std::make_shared<ImageData>(overlay);
        m_outLabelsData = std::make_shared<MarkersData>(labels);
        m_outLabelsPreviewData = std::make_shared<ImageData>(m_outLabelsData->preview());
    }

    emit dataUpdated(0);
    emit dataUpdated(1);
    emit dataUpdated(2);
    requestProcess();
}

void WatershedModel::requestProcess() {
    if (m_watcher.isRunning() || m_lastImageToProcess.isNull() || m_lastMarkersToProcess.empty()) {
        return;
    }

    const auto future = QtConcurrent::run(processImage, m_lastImageToProcess, m_lastMarkersToProcess, m_overlayAlpha);
    m_lastImageToProcess = QImage();
    m_lastMarkersToProcess.release();
    m_watcher.setFuture(future);
}

std::tuple<QImage, cv::Mat, int, quint64> WatershedModel::processImage(const QImage image, cv::Mat markers,
                                                                       const double overlayAlpha) {
    QElapsedTimer timer;
    timer.start();

    try {
        cv::Mat source = EnsureBgr(QImageToMat(image));
        if (source.empty() || markers.empty()) {
            return {QImage(), cv::Mat(), 0, timer.elapsed()};
        }

        if (markers.type() != CV_32S) {
            markers.convertTo(markers, CV_32S);
        }

        if (source.rows != markers.rows || source.cols != markers.cols) {
            qDebug() << "Watershed markers size mismatch";
            return {QImage(), cv::Mat(), 0, timer.elapsed()};
        }

        cv::watershed(source, markers);

        return {CreateOverlay(source, markers, overlayAlpha), markers, CountRegions(markers), timer.elapsed()};
    } catch (const cv::Exception& exception) {
        qDebug() << exception.what();
        return {QImage(), cv::Mat(), 0, timer.elapsed()};
    }
}

void WatershedModel::updatePendingInputs() {
    if (const auto image = m_inImageData.lock()) {
        m_lastImageToProcess = image->image();
    } else {
        m_lastImageToProcess = QImage();
    }

    if (const auto markers = m_inMarkersData.lock()) {
        m_lastMarkersToProcess = markers->markers();
    } else {
        m_lastMarkersToProcess.release();
    }

    if (m_lastImageToProcess.isNull() || m_lastMarkersToProcess.empty()) {
        m_outOverlayData.reset();
        m_outLabelsData.reset();
        m_outLabelsPreviewData.reset();
        emit dataUpdated(0);
        emit dataUpdated(1);
        emit dataUpdated(2);
        return;
    }

    requestProcess();
}
