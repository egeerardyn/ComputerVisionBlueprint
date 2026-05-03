#include "BilateralFilterModel.h"

#include <QElapsedTimer>
#include <QtConcurrent/QtConcurrent>

#include "Nodes/Conversor/MatQt.h"
#include "Nodes/OpenCV/OpenCVFilterUtils.h"
#include "ui_BilateralFilterForm.h"

BilateralFilterModel::BilateralFilterModel() {
    connect(&m_watcher, &QFutureWatcher<std::tuple<QImage, quint64>>::finished, this,
            &BilateralFilterModel::processFinished);
}

BilateralFilterModel::~BilateralFilterModel() {
}

QString BilateralFilterModel::caption() const {
    return "Bilateral Filter";
}

QString BilateralFilterModel::name() const {
    return "Bilateral Filter";
}

unsigned BilateralFilterModel::nPorts(const QtNodes::PortType portType) const {
    switch (portType) {
        case QtNodes::PortType::In:
        case QtNodes::PortType::Out:
            return 1;
        default:
            return 0;
    }
}

QtNodes::NodeDataType BilateralFilterModel::dataType(QtNodes::PortType portType, QtNodes::PortIndex portIndex) const {
    Q_UNUSED(portType)
    Q_UNUSED(portIndex)
    return ImageData().type();
}

void BilateralFilterModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex portIndex) {
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

std::shared_ptr<QtNodes::NodeData> BilateralFilterModel::outData(QtNodes::PortIndex port) {
    Q_UNUSED(port)
    return m_outImageData;
}

QWidget* BilateralFilterModel::embeddedWidget() {
    if (!m_widget) {
        m_ui = std::make_unique<Ui::BilateralFilterForm>();
        m_widget = new QWidget();
        m_ui->setupUi(m_widget);

        connect(m_ui->sb_diameter, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
            m_diameter = value;
            m_lastImageToProcess = getImageToProcess();
            requestProcess();
        });
        connect(m_ui->sb_sigmaColor, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
            m_sigmaColor = value;
            m_lastImageToProcess = getImageToProcess();
            requestProcess();
        });
        connect(m_ui->sb_sigmaSpace, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
            m_sigmaSpace = value;
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

void BilateralFilterModel::processFinished() {
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

void BilateralFilterModel::requestProcess() {
    if (m_watcher.isRunning() || m_lastImageToProcess.isNull()) {
        return;
    }

    const auto future = QtConcurrent::run(processImage, m_lastImageToProcess, m_diameter, m_sigmaColor, m_sigmaSpace,
                                          m_borderType);
    m_lastImageToProcess = QImage();
    m_watcher.setFuture(future);
}

std::tuple<QImage, quint64> BilateralFilterModel::processImage(QImage image, const int diameter,
                                                               const double sigmaColor, const double sigmaSpace,
                                                               const int borderType) {
    QElapsedTimer timer;
    timer.start();

    try {
        const cv::Mat src = QImageToMat(image);
        cv::Mat dst;
        cv::bilateralFilter(src, dst, diameter, sigmaColor, sigmaSpace, borderType);
        return {MatToDisplayImage(dst), timer.elapsed()};
    } catch (const cv::Exception& exception) {
        qDebug() << exception.what();
        return {QImage(), timer.elapsed()};
    }
}

QImage BilateralFilterModel::getImageToProcess() const {
    if (const auto lock = m_inImageData.lock()) {
        return lock->image();
    }
    return {};
}
