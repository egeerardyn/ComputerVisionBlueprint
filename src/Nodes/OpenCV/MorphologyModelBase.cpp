#include "MorphologyModelBase.h"

#include <algorithm>

#include <QAbstractSpinBox>
#include <QComboBox>
#include <QDebug>
#include <QElapsedTimer>
#include <QFormLayout>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QtConcurrent/QtConcurrent>

#include "Nodes/Conversor/MatQt.h"
#include "Nodes/OpenCV/OpenCVFilterUtils.h"

MorphologyModelBase::MorphologyModelBase(QString title, const int defaultOperation,
                                         std::vector<OperationOption> operationOptions)
    : m_title(std::move(title)),
      m_operationOptions(std::move(operationOptions)),
      m_operation(defaultOperation),
      m_useMorphologyEx(!m_operationOptions.empty()) {
    connect(&m_watcher, &QFutureWatcher<std::tuple<QImage, quint64>>::finished, this,
            &MorphologyModelBase::processFinished);
}

MorphologyModelBase::~MorphologyModelBase() {
}

QString MorphologyModelBase::caption() const {
    return m_title;
}

QString MorphologyModelBase::name() const {
    return m_title;
}

unsigned MorphologyModelBase::nPorts(const QtNodes::PortType portType) const {
    switch (portType) {
        case QtNodes::PortType::In:
        case QtNodes::PortType::Out:
            return 1;
        default:
            return 0;
    }
}

QtNodes::NodeDataType MorphologyModelBase::dataType(const QtNodes::PortType portType,
                                                    const QtNodes::PortIndex portIndex) const {
    Q_UNUSED(portType)
    Q_UNUSED(portIndex)
    return ImageData().type();
}

void MorphologyModelBase::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, const QtNodes::PortIndex portIndex) {
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

std::shared_ptr<QtNodes::NodeData> MorphologyModelBase::outData(QtNodes::PortIndex port) {
    Q_UNUSED(port)
    return m_outImageData;
}

QWidget* MorphologyModelBase::embeddedWidget() {
    if (!m_widget) {
        createWidget();
    }
    return m_widget;
}

void MorphologyModelBase::processFinished() {
    const auto [image, elapsed] = m_watcher.result();
    if (m_timeSpinBox) {
        m_timeSpinBox->setValue(static_cast<int>(elapsed));
    }
    if (m_inImageData.expired()) {
        m_outImageData.reset();
    } else {
        m_outImageData = std::make_shared<ImageData>(image);
    }
    emit dataUpdated(0);
    requestProcess();
}

void MorphologyModelBase::requestProcess() {
    if (m_watcher.isRunning() || m_lastImageToProcess.isNull()) {
        return;
    }

    const auto future = QtConcurrent::run(processImage, m_lastImageToProcess, m_kernelSize, m_kernelShape,
                                          m_iterations, m_operation, m_useMorphologyEx);
    m_lastImageToProcess = QImage();
    m_watcher.setFuture(future);
}

std::tuple<QImage, quint64> MorphologyModelBase::processImage(QImage image, const QSize kernelSize,
                                                              const int kernelShape, const int iterations,
                                                              const int operation, const bool useMorphologyEx) {
    QElapsedTimer timer;
    timer.start();

    try {
        const cv::Mat src = QImageToMat(image);
        cv::Mat dst;
        const cv::Mat kernel = cv::getStructuringElement(kernelShape,
                                                         cv::Size(std::max(1, kernelSize.width()),
                                                                  std::max(1, kernelSize.height())));

        if (useMorphologyEx) {
            cv::morphologyEx(src, dst, operation, kernel, cv::Point(-1, -1), iterations);
        } else if (operation == cv::MORPH_DILATE) {
            cv::dilate(src, dst, kernel, cv::Point(-1, -1), iterations);
        } else {
            cv::erode(src, dst, kernel, cv::Point(-1, -1), iterations);
        }

        return {MatToDisplayImage(dst), timer.elapsed()};
    } catch (const cv::Exception& exception) {
        qDebug() << exception.what();
        return {QImage(), timer.elapsed()};
    }
}

void MorphologyModelBase::updatePendingImage() {
    m_lastImageToProcess = getImageToProcess();
}

QImage MorphologyModelBase::getImageToProcess() const {
    if (const auto lock = m_inImageData.lock()) {
        return lock->image();
    }
    return {};
}

void MorphologyModelBase::createWidget() {
    m_widget = new QWidget();

    auto* rootLayout = new QVBoxLayout(m_widget);
    rootLayout->setContentsMargins(6, 6, 6, 6);

    auto* formLayout = new QFormLayout();

    m_kernelWidthSpinBox = new QSpinBox(m_widget);
    m_kernelWidthSpinBox->setRange(1, 999);
    m_kernelWidthSpinBox->setValue(m_kernelSize.width());
    formLayout->addRow("Kernel width", m_kernelWidthSpinBox);

    m_kernelHeightSpinBox = new QSpinBox(m_widget);
    m_kernelHeightSpinBox->setRange(1, 999);
    m_kernelHeightSpinBox->setValue(m_kernelSize.height());
    formLayout->addRow("Kernel height", m_kernelHeightSpinBox);

    m_shapeCombo = new QComboBox(m_widget);
    m_shapeCombo->addItems({"Rectangle", "Cross", "Ellipse"});
    formLayout->addRow("Shape", m_shapeCombo);

    m_iterationsSpinBox = new QSpinBox(m_widget);
    m_iterationsSpinBox->setRange(1, 999);
    m_iterationsSpinBox->setValue(m_iterations);
    formLayout->addRow("Iterations", m_iterationsSpinBox);

    if (m_useMorphologyEx) {
        m_operationCombo = new QComboBox(m_widget);
        int selectedIndex = 0;
        for (int i = 0; i < static_cast<int>(m_operationOptions.size()); ++i) {
            m_operationCombo->addItem(m_operationOptions[static_cast<size_t>(i)].label);
            if (m_operationOptions[static_cast<size_t>(i)].operation == m_operation) {
                selectedIndex = i;
            }
        }
        m_operationCombo->setCurrentIndex(selectedIndex);
        formLayout->addRow("Operation", m_operationCombo);
    }

    m_timeSpinBox = new QSpinBox(m_widget);
    m_timeSpinBox->setRange(0, 999999999);
    m_timeSpinBox->setReadOnly(true);
    m_timeSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
    formLayout->addRow("Time ms", m_timeSpinBox);

    rootLayout->addLayout(formLayout);

    connect(m_kernelWidthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
        m_kernelSize.setWidth(value);
        updatePendingImage();
        requestProcess();
    });
    connect(m_kernelHeightSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
        m_kernelSize.setHeight(value);
        updatePendingImage();
        requestProcess();
    });
    connect(m_shapeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        m_kernelShape = MorphShapeFromIndex(index);
        updatePendingImage();
        requestProcess();
    });
    connect(m_iterationsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
        m_iterations = value;
        updatePendingImage();
        requestProcess();
    });

    if (m_operationCombo) {
        connect(m_operationCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
            if (index < 0 || index >= static_cast<int>(m_operationOptions.size())) {
                return;
            }
            m_operation = m_operationOptions[static_cast<size_t>(index)].operation;
            updatePendingImage();
            requestProcess();
        });
    }
}
