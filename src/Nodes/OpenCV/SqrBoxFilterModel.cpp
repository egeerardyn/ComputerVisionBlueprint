#include "SqrBoxFilterModel.h"

#include <QElapsedTimer>
#include <QtConcurrent/QtConcurrent>

#include "Nodes/Conversor/MatQt.h"
#include "Nodes/OpenCV/OpenCVFilterUtils.h"
#include "ui_BoxFilterForm.h"

SqrBoxFilterModel::SqrBoxFilterModel() {
    connect(&m_watcher, &QFutureWatcher<std::tuple<QImage, quint64>>::finished, this,
            &SqrBoxFilterModel::processFinished);
}

SqrBoxFilterModel::~SqrBoxFilterModel() {
}

QString SqrBoxFilterModel::caption() const {
    return "SQR Box Filter";
}

QString SqrBoxFilterModel::name() const {
    return "SQR Box Filter";
}

unsigned SqrBoxFilterModel::nPorts(const QtNodes::PortType portType) const {
    switch (portType) {
        case QtNodes::PortType::In:
        case QtNodes::PortType::Out:
            return 1;
        default:
            return 0;
    }
}

QtNodes::NodeDataType SqrBoxFilterModel::dataType(QtNodes::PortType portType, QtNodes::PortIndex portIndex) const {
    Q_UNUSED(portType)
    Q_UNUSED(portIndex)
    return ImageData().type();
}

void SqrBoxFilterModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex portIndex) {
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

std::shared_ptr<QtNodes::NodeData> SqrBoxFilterModel::outData(QtNodes::PortIndex port) {
    Q_UNUSED(port)
    return m_outImageData;
}

QWidget* SqrBoxFilterModel::embeddedWidget() {
    if (!m_widget) {
        m_ui = std::make_unique<Ui::BoxFilterForm>();
        m_widget = new QWidget();
        m_ui->setupUi(m_widget);
        m_ui->cb_normalize->setChecked(true);

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

void SqrBoxFilterModel::processFinished() {
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

void SqrBoxFilterModel::requestProcess() {
    if (m_watcher.isRunning() || m_lastImageToProcess.isNull()) {
        return;
    }

    const auto future = QtConcurrent::run(processImage, m_lastImageToProcess, m_kernelSize, m_normalize, m_borderType);
    m_lastImageToProcess = QImage();
    m_watcher.setFuture(future);
}

std::tuple<QImage, quint64> SqrBoxFilterModel::processImage(QImage image, const QSize kernelSize, const bool normalize,
                                                            const int borderType) {
    QElapsedTimer timer;
    timer.start();

    try {
        const cv::Mat src = QImageToMat(image);
        cv::Mat dst;
        cv::sqrBoxFilter(src, dst, CV_32F, cv::Size(kernelSize.width(), kernelSize.height()), cv::Point(-1, -1),
                         normalize, borderType);
        return {MatToDisplayImage(dst), timer.elapsed()};
    } catch (const cv::Exception& exception) {
        qDebug() << exception.what();
        return {QImage(), timer.elapsed()};
    }
}

QImage SqrBoxFilterModel::getImageToProcess() const {
    if (const auto lock = m_inImageData.lock()) {
        return lock->image();
    }
    return {};
}
