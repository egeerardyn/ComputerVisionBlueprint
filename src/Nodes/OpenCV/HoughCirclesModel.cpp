#include "HoughCirclesModel.h"

#include "Nodes/NodeHelpInfo.h"
#include "Nodes/Conversor/MatQt.h"
#include "Nodes/OpenCV/HoughCirclesUtils.h"
#include "Nodes/OpenCV/OpenCVFilterUtils.h"

#include <QAbstractSpinBox>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
#include <QFormLayout>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QtConcurrent/QtConcurrent>

namespace {
const NodeHelpRegistration kHoughCirclesModelHelp(QStringLiteral("Hough Circles"),
                                                  makeNodeHelp(QStringLiteral("Detects circles with the Hough gradient transform and outputs circle center/radius data."),
                                                               QStringLiteral("https://docs.opencv.org/4.x/d4/d70/tutorial_hough_circle.html")));
} // namespace

HoughCirclesModel::HoughCirclesModel() {
    connect(&m_watcher, &QFutureWatcher<std::tuple<CirclesData, quint64>>::finished, this,
            &HoughCirclesModel::processFinished);
}

HoughCirclesModel::~HoughCirclesModel() {
}

QString HoughCirclesModel::caption() const {
    return "Hough Circles";
}

QString HoughCirclesModel::name() const {
    return "Hough Circles";
}

unsigned HoughCirclesModel::nPorts(const QtNodes::PortType portType) const {
    switch (portType) {
        case QtNodes::PortType::In:
            return 1;
        case QtNodes::PortType::Out:
            return 1;
        default:
            return 0;
    }
}

QtNodes::NodeDataType HoughCirclesModel::dataType(const QtNodes::PortType portType,
                                                  const QtNodes::PortIndex portIndex) const {
    Q_UNUSED(portIndex)
    return portType == QtNodes::PortType::In ? ImageData().type() : CirclesData().type();
}

void HoughCirclesModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, const QtNodes::PortIndex portIndex) {
    Q_UNUSED(portIndex)

    m_inImageData = std::dynamic_pointer_cast<ImageData>(nodeData);
    if (const auto lock = m_inImageData.lock()) {
        m_lastImageToProcess = lock->image();
    } else {
        m_lastImageToProcess = QImage();
        m_outCirclesData.reset();
        emit dataUpdated(0);
    }

    requestProcess();
}

std::shared_ptr<QtNodes::NodeData> HoughCirclesModel::outData(const QtNodes::PortIndex port) {
    Q_UNUSED(port)
    return m_outCirclesData;
}

QWidget* HoughCirclesModel::embeddedWidget() {
    if (!m_widget) {
        m_widget = new QWidget();
        auto* rootLayout = new QVBoxLayout(m_widget);
        rootLayout->setContentsMargins(6, 6, 6, 6);

        auto* formLayout = new QFormLayout();

        m_dpSpinBox = new QDoubleSpinBox(m_widget);
        m_dpSpinBox->setRange(1.0, 10.0);
        m_dpSpinBox->setSingleStep(0.1);
        m_dpSpinBox->setValue(m_dp);
        formLayout->addRow("dp", m_dpSpinBox);

        m_minDistSpinBox = new QDoubleSpinBox(m_widget);
        m_minDistSpinBox->setRange(1.0, 2048.0);
        m_minDistSpinBox->setSingleStep(1.0);
        m_minDistSpinBox->setValue(m_minDist);
        formLayout->addRow("Min dist", m_minDistSpinBox);

        m_param1SpinBox = new QDoubleSpinBox(m_widget);
        m_param1SpinBox->setRange(1.0, 1024.0);
        m_param1SpinBox->setSingleStep(1.0);
        m_param1SpinBox->setValue(m_param1);
        formLayout->addRow("Param 1", m_param1SpinBox);

        m_param2SpinBox = new QDoubleSpinBox(m_widget);
        m_param2SpinBox->setRange(1.0, 1024.0);
        m_param2SpinBox->setSingleStep(1.0);
        m_param2SpinBox->setValue(m_param2);
        formLayout->addRow("Param 2", m_param2SpinBox);

        m_minRadiusSpinBox = new QSpinBox(m_widget);
        m_minRadiusSpinBox->setRange(0, 4096);
        m_minRadiusSpinBox->setValue(m_minRadius);
        formLayout->addRow("Min radius", m_minRadiusSpinBox);

        m_maxRadiusSpinBox = new QSpinBox(m_widget);
        m_maxRadiusSpinBox->setRange(0, 4096);
        m_maxRadiusSpinBox->setValue(m_maxRadius);
        formLayout->addRow("Max radius", m_maxRadiusSpinBox);

        m_timeSpinBox = new QSpinBox(m_widget);
        m_timeSpinBox->setRange(0, 999999999);
        m_timeSpinBox->setReadOnly(true);
        m_timeSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
        formLayout->addRow("Time ms", m_timeSpinBox);

        rootLayout->addLayout(formLayout);

        connect(m_dpSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
            m_dp = value;
            m_lastImageToProcess = getImageToProcess();
            requestProcess();
        });
        connect(m_minDistSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
            m_minDist = value;
            m_lastImageToProcess = getImageToProcess();
            requestProcess();
        });
        connect(m_param1SpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
            m_param1 = value;
            m_lastImageToProcess = getImageToProcess();
            requestProcess();
        });
        connect(m_param2SpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
            m_param2 = value;
            m_lastImageToProcess = getImageToProcess();
            requestProcess();
        });
        connect(m_minRadiusSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
            m_minRadius = value;
            m_lastImageToProcess = getImageToProcess();
            requestProcess();
        });
        connect(m_maxRadiusSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
            m_maxRadius = value;
            m_lastImageToProcess = getImageToProcess();
            requestProcess();
        });
    }

    return m_widget;
}

bool HoughCirclesModel::portCaptionVisible(QtNodes::PortType, QtNodes::PortIndex) const {
    return true;
}

QString HoughCirclesModel::portCaption(const QtNodes::PortType portType, const QtNodes::PortIndex portIndex) const {
    Q_UNUSED(portIndex)
    return portType == QtNodes::PortType::In ? QStringLiteral("Image") : QStringLiteral("Circles");
}

void HoughCirclesModel::processFinished() {
    const auto [circlesData, elapsed] = m_watcher.result();
    if (m_inImageData.expired()) {
        m_outCirclesData.reset();
    } else {
        m_outCirclesData = std::make_shared<CirclesData>(circlesData);
    }

    if (m_timeSpinBox) {
        m_timeSpinBox->setValue(static_cast<int>(elapsed));
    }

    emit dataUpdated(0);
    requestProcess();
}

void HoughCirclesModel::requestProcess() {
    if (m_watcher.isRunning()) {
        return;
    }

    if (m_lastImageToProcess.isNull()) {
        return;
    }

    const auto future = QtConcurrent::run(processImage,
                                          m_lastImageToProcess,
                                          m_dp,
                                          m_minDist,
                                          m_param1,
                                          m_param2,
                                          m_minRadius,
                                          m_maxRadius);
    m_lastImageToProcess = QImage();
    m_watcher.setFuture(future);
}

std::tuple<CirclesData, quint64> HoughCirclesModel::processImage(const QImage image,
                                                                 const double dp,
                                                                 const double minDist,
                                                                 const double param1,
                                                                 const double param2,
                                                                 const int minRadius,
                                                                 const int maxRadius) {
    QElapsedTimer timer;
    timer.start();

    const cv::Mat source = QImageToMat(image);
    if (source.empty()) {
        return {CirclesData(), timer.elapsed()};
    }

    try {
        const std::vector<cv::Vec3f> circles = DetectHoughCircles(source, dp, minDist, param1, param2, minRadius, maxRadius);
        return {CirclesData(circles), timer.elapsed()};
    } catch (const cv::Exception& exception) {
        LogOpenCvError("HoughCircles", exception);
    } catch (const std::exception& exception) {
        LogStdError("HoughCircles", exception);
    } catch (...) {
        LogUnknownError("HoughCircles");
    }

    return {CirclesData(), timer.elapsed()};
}

QImage HoughCirclesModel::getImageToProcess() const {
    if (const auto lock = m_inImageData.lock()) {
        return lock->image();
    }

    return {};
}
