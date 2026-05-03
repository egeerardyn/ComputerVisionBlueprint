#include "DistanceTransformModel.h"
#include "Nodes/NodeHelpInfo.h"

#include <QAbstractSpinBox>
#include <QComboBox>
#include <QDebug>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QtConcurrent/QtConcurrent>

#include "Nodes/Conversor/MatQt.h"
#include "Nodes/OpenCV/OpenCVFilterUtils.h"

namespace {
const NodeHelpRegistration kDistanceTransformModelHelp(QStringLiteral("Distance Transform"),
                                                       makeNodeHelp(QStringLiteral("Computes the distance from each foreground pixel to the nearest background pixel, which is often used before watershed segmentation."),
                                                                    QStringLiteral("https://docs.opencv.org/4.x/d7/d1b/group__imgproc__misc.html")));

int DistanceTypeFromIndex(const int index) {
    switch (index) {
        case 0:
            return cv::DIST_L1;
        case 1:
            return cv::DIST_L2;
        case 2:
            return cv::DIST_C;
        default:
            return cv::DIST_L2;
    }
}

int MaskSizeFromIndex(const int index) {
    switch (index) {
        case 0:
            return cv::DIST_MASK_3;
        case 1:
            return cv::DIST_MASK_5;
        case 2:
            return cv::DIST_MASK_PRECISE;
        default:
            return cv::DIST_MASK_3;
    }
}

cv::Mat EnsureGray(const cv::Mat& src) {
    if (src.channels() == 1) {
        return src;
    }

    cv::Mat gray;
    if (src.channels() == 4) {
        cv::cvtColor(src, gray, cv::COLOR_BGRA2GRAY);
    } else {
        cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    }
    return gray;
}
} // namespace

DistanceTransformModel::DistanceTransformModel() {
    connect(&m_watcher, &QFutureWatcher<std::tuple<QImage, cv::Mat, quint64>>::finished, this,
            &DistanceTransformModel::processFinished);
}

DistanceTransformModel::~DistanceTransformModel() {
}

QString DistanceTransformModel::caption() const {
    return "Distance Transform";
}

QString DistanceTransformModel::name() const {
    return "Distance Transform";
}

unsigned DistanceTransformModel::nPorts(const QtNodes::PortType portType) const {
    switch (portType) {
        case QtNodes::PortType::In:
            return 1;
        case QtNodes::PortType::Out:
            return 3;
        default:
            return 0;
    }
}

QtNodes::NodeDataType DistanceTransformModel::dataType(const QtNodes::PortType portType,
                                                       const QtNodes::PortIndex portIndex) const {
    switch (portType) {
        case QtNodes::PortType::In:
            return ImageData().type();
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

void DistanceTransformModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, const QtNodes::PortIndex portIndex) {
    Q_UNUSED(portIndex)

    m_inImageData = std::dynamic_pointer_cast<ImageData>(nodeData);
    if (const auto lock = m_inImageData.lock()) {
        m_lastImageToProcess = lock->image();
    } else {
        m_lastImageToProcess = QImage();
        m_outDistanceImageData.reset();
        m_outMarkersData.reset();
        m_outMarkersPreviewData.reset();
        emit dataUpdated(0);
        emit dataUpdated(1);
        emit dataUpdated(2);
    }

    requestProcess();
}

std::shared_ptr<QtNodes::NodeData> DistanceTransformModel::outData(QtNodes::PortIndex port) {
    switch (port) {
        case 0:
            return m_outDistanceImageData;
        case 1:
            return m_outMarkersData;
        case 2:
            return m_outMarkersPreviewData;
        default:
            return nullptr;
    }
}

QWidget* DistanceTransformModel::embeddedWidget() {
    if (!m_widget) {
        m_widget = new QWidget();
        auto* rootLayout = new QVBoxLayout(m_widget);
        rootLayout->setContentsMargins(6, 6, 6, 6);

        auto* formLayout = new QFormLayout();

        m_distanceTypeCombo = new QComboBox(m_widget);
        m_distanceTypeCombo->addItems({"L1", "L2", "Chessboard"});
        m_distanceTypeCombo->setCurrentIndex(1);
        formLayout->addRow("Distance", m_distanceTypeCombo);

        m_maskSizeCombo = new QComboBox(m_widget);
        m_maskSizeCombo->addItems({"3", "5", "Precise"});
        formLayout->addRow("Mask", m_maskSizeCombo);

        m_markerThresholdSpinBox = new QDoubleSpinBox(m_widget);
        m_markerThresholdSpinBox->setRange(0.05, 0.95);
        m_markerThresholdSpinBox->setSingleStep(0.05);
        m_markerThresholdSpinBox->setDecimals(2);
        m_markerThresholdSpinBox->setValue(m_markerThreshold);
        formLayout->addRow("Peak threshold", m_markerThresholdSpinBox);

        m_timeSpinBox = new QDoubleSpinBox(m_widget);
        m_timeSpinBox->setRange(0.0, 999999999.0);
        m_timeSpinBox->setReadOnly(true);
        m_timeSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
        formLayout->addRow("Time ms", m_timeSpinBox);

        rootLayout->addLayout(formLayout);

        connect(m_distanceTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
            m_distanceType = DistanceTypeFromIndex(index);
            m_lastImageToProcess = getImageToProcess();
            requestProcess();
        });
        connect(m_maskSizeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
            m_maskSize = MaskSizeFromIndex(index);
            m_lastImageToProcess = getImageToProcess();
            requestProcess();
        });
        connect(m_markerThresholdSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
                [this](double value) {
                    m_markerThreshold = value;
                    m_lastImageToProcess = getImageToProcess();
                    requestProcess();
                });
    }

    return m_widget;
}

bool DistanceTransformModel::portCaptionVisible(QtNodes::PortType, QtNodes::PortIndex) const {
    return true;
}

QString DistanceTransformModel::portCaption(const QtNodes::PortType portType, const QtNodes::PortIndex portIndex) const {
    switch (portType) {
        case QtNodes::PortType::In:
            return "Image";
        case QtNodes::PortType::Out:
            switch (portIndex) {
                case 0:
                    return "Distance";
                case 1:
                    return "Markers";
                case 2:
                    return "Marker Preview";
                default:
                    return {};
            }
        default:
            return NodeDelegateModel::portCaption(portType, portIndex);
    }
}

void DistanceTransformModel::processFinished() {
    const auto [distanceImage, markers, elapsed] = m_watcher.result();
    if (m_timeSpinBox) {
        m_timeSpinBox->setValue(static_cast<double>(elapsed));
    }

    if (m_inImageData.expired()) {
        m_outDistanceImageData.reset();
        m_outMarkersData.reset();
        m_outMarkersPreviewData.reset();
    } else {
        m_outDistanceImageData = std::make_shared<ImageData>(distanceImage);
        m_outMarkersData = std::make_shared<MarkersData>(markers);
        m_outMarkersPreviewData = std::make_shared<ImageData>(m_outMarkersData->preview());
    }

    emit dataUpdated(0);
    emit dataUpdated(1);
    emit dataUpdated(2);
    requestProcess();
}

void DistanceTransformModel::requestProcess() {
    if (m_watcher.isRunning() || m_lastImageToProcess.isNull()) {
        return;
    }

    const auto future = QtConcurrent::run(processImage, m_lastImageToProcess, m_distanceType, m_maskSize,
                                          m_markerThreshold);
    m_lastImageToProcess = QImage();
    m_watcher.setFuture(future);
}

std::tuple<QImage, cv::Mat, quint64> DistanceTransformModel::processImage(const QImage image, const int distanceType,
                                                                          const int maskSize,
                                                                          const double markerThreshold) {
    QElapsedTimer timer;
    timer.start();

    try {
        cv::Mat gray = EnsureGray(QImageToMat(image));
        if (gray.empty()) {
            return {QImage(), cv::Mat(), timer.elapsed()};
        }

        cv::GaussianBlur(gray, gray, cv::Size(3, 3), 0.0);

        cv::Mat binary;
        cv::threshold(gray, binary, 0.0, 255.0, cv::THRESH_BINARY | cv::THRESH_OTSU);
        if (cv::countNonZero(binary) > (binary.rows * binary.cols) / 2) {
            cv::bitwise_not(binary, binary);
        }

        cv::Mat distance;
        cv::distanceTransform(binary, distance, distanceType, maskSize);

        cv::Mat normalizedDistance;
        cv::normalize(distance, normalizedDistance, 0.0, 1.0, cv::NORM_MINMAX);

        cv::Mat sureForeground;
        cv::threshold(normalizedDistance, sureForeground, markerThreshold, 1.0, cv::THRESH_BINARY);

        cv::Mat sureForeground8;
        sureForeground.convertTo(sureForeground8, CV_8U, 255.0);

        cv::Mat sureBackground;
        cv::dilate(binary, sureBackground, cv::Mat(), cv::Point(-1, -1), 3);

        cv::Mat unknown;
        cv::subtract(sureBackground, sureForeground8, unknown);

        cv::Mat markers;
        cv::connectedComponents(sureForeground8, markers);
        markers += 1;
        markers.setTo(0, unknown == 255);

        return {MatToDisplayImage(normalizedDistance), markers, timer.elapsed()};
    } catch (const cv::Exception& exception) {
        qDebug() << exception.what();
        return {QImage(), cv::Mat(), timer.elapsed()};
    }
}

QImage DistanceTransformModel::getImageToProcess() const {
    if (const auto lock = m_inImageData.lock()) {
        return lock->image();
    }
    return {};
}
