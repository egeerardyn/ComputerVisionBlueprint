#include "MedianBlurModel.h"

#include <QElapsedTimer>
#include <QtConcurrent/QtConcurrent>

#include "Nodes/Conversor/MatQt.h"
#include "Nodes/OpenCV/OpenCVFilterUtils.h"
#include "ui_MedianBlurForm.h"

MedianBlurModel::MedianBlurModel() {
    connect(&m_watcher, &QFutureWatcher<std::tuple<QImage, quint64>>::finished, this, &MedianBlurModel::processFinished);
}

MedianBlurModel::~MedianBlurModel() {
}

QString MedianBlurModel::caption() const {
    return "Median Blur";
}

QString MedianBlurModel::name() const {
    return "Median Blur";
}

unsigned MedianBlurModel::nPorts(const QtNodes::PortType portType) const {
    switch (portType) {
        case QtNodes::PortType::In:
        case QtNodes::PortType::Out:
            return 1;
        default:
            return 0;
    }
}

QtNodes::NodeDataType MedianBlurModel::dataType(const QtNodes::PortType portType, const QtNodes::PortIndex portIndex) const {
    Q_UNUSED(portType)
    Q_UNUSED(portIndex)
    return ImageData().type();
}

void MedianBlurModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, const QtNodes::PortIndex portIndex) {
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

std::shared_ptr<QtNodes::NodeData> MedianBlurModel::outData(const QtNodes::PortIndex port) {
    Q_UNUSED(port)
    return m_outImageData;
}

QWidget* MedianBlurModel::embeddedWidget() {
    if (!m_widget) {
        m_ui = std::make_unique<Ui::MedianBlurForm>();
        m_widget = new QWidget();
        m_ui->setupUi(m_widget);
        connect(m_ui->sb_kernelSize, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
            m_kernelSize = value % 2 == 0 ? value + 1 : value;
            if (m_ui->sb_kernelSize->value() != m_kernelSize) {
                m_ui->sb_kernelSize->setValue(m_kernelSize);
                return;
            }
            m_lastImageToProcess = getImageToProcess();
            requestProcess();
        });
    }
    return m_widget;
}

void MedianBlurModel::processFinished() {
    const auto [image, elapsed] = m_watcher.result();
    if (m_ui) {
        m_ui->sb_time->setValue(static_cast<int>(elapsed));
    }
    if (m_inImageData.expired()) {
        m_outImageData.reset();
    } else {
        m_outImageData = std::make_shared<ImageData>(image);
    }
    emit dataUpdated(0);
    requestProcess();
}

void MedianBlurModel::requestProcess() {
    if (m_watcher.isRunning() || m_lastImageToProcess.isNull()) {
        return;
    }

    const auto future = QtConcurrent::run(processImage, m_lastImageToProcess, m_kernelSize);
    m_lastImageToProcess = QImage();
    m_watcher.setFuture(future);
}

std::tuple<QImage, quint64> MedianBlurModel::processImage(QImage image, const int kernelSize) {
    QElapsedTimer timer;
    timer.start();

    try {
        const cv::Mat src = QImageToMat(image);
        cv::Mat dst;
        cv::medianBlur(src, dst, kernelSize);
        return {MatToDisplayImage(dst), timer.elapsed()};
    } catch (const cv::Exception& exception) {
        qDebug() << exception.what();
        return {QImage(), timer.elapsed()};
    }
}

QImage MedianBlurModel::getImageToProcess() const {
    if (const auto lock = m_inImageData.lock()) {
        return lock->image();
    }
    return {};
}
