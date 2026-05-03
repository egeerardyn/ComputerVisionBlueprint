#include "BoxFilterModel.h"

#include <QElapsedTimer>
#include <QtConcurrent/QtConcurrent>

#include "Nodes/Conversor/MatQt.h"
#include "Nodes/OpenCV/OpenCVFilterUtils.h"
#include "ui_BoxFilterForm.h"

BoxFilterModel::BoxFilterModel() {
    connect(&m_watcher, &QFutureWatcher<std::tuple<QImage, quint64>>::finished, this, &BoxFilterModel::processFinished);
}

BoxFilterModel::~BoxFilterModel() {
}

QString BoxFilterModel::caption() const {
    return "Box Filter";
}

QString BoxFilterModel::name() const {
    return "Box Filter";
}

unsigned BoxFilterModel::nPorts(const QtNodes::PortType portType) const {
    switch (portType) {
        case QtNodes::PortType::In:
        case QtNodes::PortType::Out:
            return 1;
        default:
            return 0;
    }
}

QtNodes::NodeDataType BoxFilterModel::dataType(QtNodes::PortType portType, QtNodes::PortIndex portIndex) const {
    Q_UNUSED(portType)
    Q_UNUSED(portIndex)
    return ImageData().type();
}

void BoxFilterModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex portIndex) {
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

std::shared_ptr<QtNodes::NodeData> BoxFilterModel::outData(QtNodes::PortIndex port) {
    Q_UNUSED(port)
    return m_outImageData;
}

QWidget* BoxFilterModel::embeddedWidget() {
    if (!m_widget) {
        m_ui = std::make_unique<Ui::BoxFilterForm>();
        m_widget = new QWidget();
        m_ui->setupUi(m_widget);

        connect(m_ui->sb_width, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
            m_kernelSize.setWidth(value);
            m_lastImageToProcess = getImageToProcess();
            requestProcess();
        });
        connect(m_ui->sb_height, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
            m_kernelSize.setHeight(value);
            m_lastImageToProcess = getImageToProcess();
            requestProcess();
        });
        connect(m_ui->cb_normalize, &QCheckBox::checkStateChanged, this, [this](Qt::CheckState state) {
            m_normalize = state == Qt::Checked;
            m_lastImageToProcess = getImageToProcess();
            requestProcess();
        });
        connect(m_ui->cb_borderType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
            m_borderType = BorderTypeFromIndex(index);
            m_lastImageToProcess = getImageToProcess();
            requestProcess();
        });
    }
    return m_widget;
}

void BoxFilterModel::processFinished() {
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

void BoxFilterModel::requestProcess() {
    if (m_watcher.isRunning() || m_lastImageToProcess.isNull()) {
        return;
    }

    const auto future = QtConcurrent::run(processImage, m_lastImageToProcess, m_kernelSize, m_normalize, m_borderType);
    m_lastImageToProcess = QImage();
    m_watcher.setFuture(future);
}

std::tuple<QImage, quint64> BoxFilterModel::processImage(QImage image, const QSize kernelSize, const bool normalize,
                                                         const int borderType) {
    QElapsedTimer timer;
    timer.start();

    try {
        const cv::Mat src = QImageToMat(image);
        cv::Mat dst;
        cv::boxFilter(src, dst, -1, cv::Size(kernelSize.width(), kernelSize.height()), cv::Point(-1, -1), normalize,
                      borderType);
        return {MatToDisplayImage(dst), timer.elapsed()};
    } catch (const cv::Exception& exception) {
        qDebug() << exception.what();
        return {QImage(), timer.elapsed()};
    }
}

QImage BoxFilterModel::getImageToProcess() const {
    if (const auto lock = m_inImageData.lock()) {
        return lock->image();
    }
    return {};
}
