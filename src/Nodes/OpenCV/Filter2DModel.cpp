#include "Filter2DModel.h"

#include <QAbstractSpinBox>
#include <QComboBox>
#include <QDebug>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSignalBlocker>
#include <QVBoxLayout>
#include <QtConcurrent/QtConcurrent>

#include "Nodes/Conversor/MatQt.h"
#include "Nodes/OpenCV/OpenCVFilterUtils.h"

Filter2DModel::Filter2DModel() {
    connect(&m_watcher, &QFutureWatcher<std::tuple<QImage, quint64>>::finished, this, &Filter2DModel::processFinished);
}

Filter2DModel::~Filter2DModel() {
}

QString Filter2DModel::caption() const {
    return "Filter2D";
}

QString Filter2DModel::name() const {
    return "Filter2D";
}

unsigned Filter2DModel::nPorts(const QtNodes::PortType portType) const {
    switch (portType) {
        case QtNodes::PortType::In:
        case QtNodes::PortType::Out:
            return 1;
        default:
            return 0;
    }
}

QtNodes::NodeDataType Filter2DModel::dataType(QtNodes::PortType portType, QtNodes::PortIndex portIndex) const {
    Q_UNUSED(portType)
    Q_UNUSED(portIndex)
    return ImageData().type();
}

void Filter2DModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex portIndex) {
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

std::shared_ptr<QtNodes::NodeData> Filter2DModel::outData(QtNodes::PortIndex port) {
    Q_UNUSED(port)
    return m_outImageData;
}

QWidget* Filter2DModel::embeddedWidget() {
    if (!m_widget) {
        m_widget = new QWidget();

        auto* rootLayout = new QVBoxLayout(m_widget);
        rootLayout->setContentsMargins(6, 6, 6, 6);

        auto* presetLayout = new QFormLayout();
        m_presetCombo = new QComboBox(m_widget);
        m_presetCombo->addItems({"Custom", "Identity", "Sharpen", "Edge Detect", "Box Blur", "Gaussian Blur"});
        presetLayout->addRow("Preset", m_presetCombo);
        rootLayout->addLayout(presetLayout);

        auto* kernelGroup = new QGroupBox("3x3 kernel", m_widget);
        auto* kernelLayout = new QGridLayout(kernelGroup);
        for (int i = 0; i < static_cast<int>(m_kernelSpinBoxes.size()); ++i) {
            auto* spinBox = new QDoubleSpinBox(kernelGroup);
            spinBox->setRange(-999.0, 999.0);
            spinBox->setDecimals(3);
            spinBox->setSingleStep(0.1);
            kernelLayout->addWidget(spinBox, i / 3, i % 3);
            m_kernelSpinBoxes[static_cast<size_t>(i)] = spinBox;
            connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this]() {
                if (m_presetCombo->currentIndex() != 0) {
                    QSignalBlocker blocker(m_presetCombo);
                    m_presetCombo->setCurrentIndex(0);
                }
                syncKernelFromUi();
                m_lastImageToProcess = getImageToProcess();
                requestProcess();
            });
        }
        rootLayout->addWidget(kernelGroup);

        auto* formLayout = new QFormLayout();
        m_deltaSpinBox = new QDoubleSpinBox(m_widget);
        m_deltaSpinBox->setRange(-9999.0, 9999.0);
        m_deltaSpinBox->setDecimals(3);
        formLayout->addRow("Delta", m_deltaSpinBox);

        m_timeSpinBox = new QDoubleSpinBox(m_widget);
        m_timeSpinBox->setRange(0.0, 999999999.0);
        m_timeSpinBox->setReadOnly(true);
        m_timeSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
        formLayout->addRow("Time ms", m_timeSpinBox);
        rootLayout->addLayout(formLayout);

        connect(m_presetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Filter2DModel::applyPreset);
        connect(m_deltaSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
            m_delta = value;
            m_lastImageToProcess = getImageToProcess();
            requestProcess();
        });

        applyPreset(1);
    }
    return m_widget;
}

void Filter2DModel::processFinished() {
    const auto [image, elapsed] = m_watcher.result();
    if (m_timeSpinBox) {
        m_timeSpinBox->setValue(static_cast<double>(elapsed));
    }
    if (m_inImageData.expired()) {
        m_outImageData.reset();
    } else {
        m_outImageData = std::make_shared<ImageData>(image);
    }
    emit dataUpdated(0);
    requestProcess();
}

void Filter2DModel::requestProcess() {
    if (m_watcher.isRunning() || m_lastImageToProcess.isNull()) {
        return;
    }

    const auto future = QtConcurrent::run(processImage, m_lastImageToProcess, m_kernelValues, m_delta);
    m_lastImageToProcess = QImage();
    m_watcher.setFuture(future);
}

std::tuple<QImage, quint64> Filter2DModel::processImage(QImage image, const std::array<double, 9> kernelValues,
                                                        const double delta) {
    QElapsedTimer timer;
    timer.start();

    try {
        const cv::Mat src = QImageToMat(image);
        cv::Mat dst;
        cv::Mat kernel(3, 3, CV_32F);
        for (int i = 0; i < 9; ++i) {
            kernel.at<float>(i / 3, i % 3) = static_cast<float>(kernelValues[static_cast<size_t>(i)]);
        }
        cv::filter2D(src, dst, CV_32F, kernel, cv::Point(-1, -1), delta, cv::BORDER_DEFAULT);
        return {MatToDisplayImage(dst), timer.elapsed()};
    } catch (const cv::Exception& exception) {
        qDebug() << exception.what();
        return {QImage(), timer.elapsed()};
    }
}

void Filter2DModel::applyPreset(const int presetIndex) {
    std::array<double, 9> presetValues = m_kernelValues;

    switch (presetIndex) {
        case 1:
            presetValues = {0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0};
            break;
        case 2:
            presetValues = {0.0, -1.0, 0.0, -1.0, 5.0, -1.0, 0.0, -1.0, 0.0};
            break;
        case 3:
            presetValues = {-1.0, -1.0, -1.0, -1.0, 8.0, -1.0, -1.0, -1.0, -1.0};
            break;
        case 4:
            presetValues = {1.0 / 9.0, 1.0 / 9.0, 1.0 / 9.0, 1.0 / 9.0, 1.0 / 9.0, 1.0 / 9.0, 1.0 / 9.0,
                            1.0 / 9.0, 1.0 / 9.0};
            break;
        case 5:
            presetValues = {1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0, 2.0 / 16.0, 4.0 / 16.0, 2.0 / 16.0,
                            1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0};
            break;
        default:
            presetValues = m_kernelValues;
            break;
    }

    for (int i = 0; i < 9; ++i) {
        QSignalBlocker blocker(m_kernelSpinBoxes[static_cast<size_t>(i)]);
        m_kernelSpinBoxes[static_cast<size_t>(i)]->setValue(presetValues[static_cast<size_t>(i)]);
    }
    syncKernelFromUi();
    m_lastImageToProcess = getImageToProcess();
    requestProcess();
}

void Filter2DModel::syncKernelFromUi() {
    for (int i = 0; i < 9; ++i) {
        m_kernelValues[static_cast<size_t>(i)] = m_kernelSpinBoxes[static_cast<size_t>(i)]->value();
    }
}

QImage Filter2DModel::getImageToProcess() const {
    if (const auto lock = m_inImageData.lock()) {
        return lock->image();
    }
    return {};
}
