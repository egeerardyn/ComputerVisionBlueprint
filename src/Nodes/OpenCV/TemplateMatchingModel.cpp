#include "TemplateMatchingModel.h"

#include "Nodes/NodeHelpInfo.h"
#include "Nodes/Conversor/MatQt.h"
#include "Nodes/OpenCV/OpenCVFilterUtils.h"

#include <QAbstractSpinBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
#include <QFormLayout>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QtConcurrent/QtConcurrent>

namespace {
const NodeHelpRegistration kTemplateMatchingModelHelp(QStringLiteral("Template Matching"),
                                                      makeNodeHelp(QStringLiteral("Matches a template image against the source image using a selectable OpenCV matching method."),
                                                                   QStringLiteral("https://docs.opencv.org/4.x/de/da9/tutorial_template_matching.html")));

int MethodFromIndex(const int index) {
    switch (index) {
        case 0:
            return cv::TM_SQDIFF;
        case 1:
            return cv::TM_SQDIFF_NORMED;
        case 2:
            return cv::TM_CCORR;
        case 3:
            return cv::TM_CCORR_NORMED;
        case 4:
            return cv::TM_CCOEFF;
        case 5:
            return cv::TM_CCOEFF_NORMED;
        default:
            return cv::TM_CCOEFF_NORMED;
    }
}
} // namespace

TemplateMatchingModel::TemplateMatchingModel() {
    connect(&m_watcher, &QFutureWatcher<std::tuple<QImage, quint64, double>>::finished, this,
            &TemplateMatchingModel::processFinished);
}

TemplateMatchingModel::~TemplateMatchingModel() {
}

QString TemplateMatchingModel::caption() const {
    return "Template Matching";
}

QString TemplateMatchingModel::name() const {
    return "Template Matching";
}

unsigned TemplateMatchingModel::nPorts(const QtNodes::PortType portType) const {
    switch (portType) {
        case QtNodes::PortType::In:
            return 2;
        case QtNodes::PortType::Out:
            return 1;
        default:
            return 0;
    }
}

QtNodes::NodeDataType TemplateMatchingModel::dataType(const QtNodes::PortType portType,
                                                      const QtNodes::PortIndex portIndex) const {
    Q_UNUSED(portIndex)
    return portType == QtNodes::PortType::Out ? ImageData().type() : ImageData().type();
}

void TemplateMatchingModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, const QtNodes::PortIndex portIndex) {
    switch (portIndex) {
        case 0:
            m_inImageData = std::dynamic_pointer_cast<ImageData>(nodeData);
            if (const auto lock = m_inImageData.lock()) {
                m_lastImageToProcess = lock->image();
            } else {
                m_lastImageToProcess = QImage();
            }
            break;
        case 1:
            m_inTemplateData = std::dynamic_pointer_cast<ImageData>(nodeData);
            if (const auto lock = m_inTemplateData.lock()) {
                m_lastTemplateToProcess = lock->image();
            } else {
                m_lastTemplateToProcess = QImage();
            }
            break;
        default:
            break;
    }

    if (m_lastImageToProcess.isNull() || m_lastTemplateToProcess.isNull()) {
        m_outImageData.reset();
        emit dataUpdated(0);
    }

    requestProcess();
}

std::shared_ptr<QtNodes::NodeData> TemplateMatchingModel::outData(const QtNodes::PortIndex port) {
    Q_UNUSED(port)
    return m_outImageData;
}

QWidget* TemplateMatchingModel::embeddedWidget() {
    if (!m_widget) {
        m_widget = new QWidget();
        auto* rootLayout = new QVBoxLayout(m_widget);
        rootLayout->setContentsMargins(6, 6, 6, 6);

        auto* formLayout = new QFormLayout();

        m_methodCombo = new QComboBox(m_widget);
        m_methodCombo->addItems({"TM_SQDIFF", "TM_SQDIFF_NORMED", "TM_CCORR", "TM_CCORR_NORMED", "TM_CCOEFF", "TM_CCOEFF_NORMED"});
        m_methodCombo->setCurrentIndex(5);
        formLayout->addRow("Method", m_methodCombo);

        m_thicknessSpinBox = new QSpinBox(m_widget);
        m_thicknessSpinBox->setRange(1, 32);
        m_thicknessSpinBox->setValue(m_thickness);
        formLayout->addRow("Box thickness", m_thicknessSpinBox);

        m_scoreSpinBox = new QDoubleSpinBox(m_widget);
        m_scoreSpinBox->setRange(-1.0, 1.0);
        m_scoreSpinBox->setDecimals(6);
        m_scoreSpinBox->setReadOnly(true);
        m_scoreSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
        formLayout->addRow("Score", m_scoreSpinBox);

        m_timeSpinBox = new QSpinBox(m_widget);
        m_timeSpinBox->setRange(0, 999999999);
        m_timeSpinBox->setReadOnly(true);
        m_timeSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
        formLayout->addRow("Time ms", m_timeSpinBox);

        rootLayout->addLayout(formLayout);

        connect(m_methodCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
            m_method = MethodFromIndex(index);
            requestProcess();
        });
        connect(m_thicknessSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
            m_thickness = value;
            requestProcess();
        });
    }

    return m_widget;
}

bool TemplateMatchingModel::portCaptionVisible(QtNodes::PortType, QtNodes::PortIndex) const {
    return true;
}

QString TemplateMatchingModel::portCaption(const QtNodes::PortType portType, const QtNodes::PortIndex portIndex) const {
    if (portType == QtNodes::PortType::In) {
        return portIndex == 0 ? "Image" : "Template";
    }

    return "Match";
}

void TemplateMatchingModel::processFinished() {
    const auto [image, elapsed, score] = m_watcher.result();
    if (m_inImageData.expired() || m_inTemplateData.expired()) {
        m_outImageData.reset();
    } else {
        m_outImageData = std::make_shared<ImageData>(image);
    }

    if (m_timeSpinBox) {
        m_timeSpinBox->setValue(static_cast<int>(elapsed));
    }
    if (m_scoreSpinBox) {
        m_scoreSpinBox->setValue(score);
    }

    emit dataUpdated(0);
    requestProcess();
}

void TemplateMatchingModel::requestProcess() {
    if (m_watcher.isRunning()) {
        return;
    }

    if (m_lastImageToProcess.isNull() || m_lastTemplateToProcess.isNull()) {
        return;
    }

    const auto future = QtConcurrent::run(processImage,
                                          m_lastImageToProcess,
                                          m_lastTemplateToProcess,
                                          m_method,
                                          m_thickness);
    m_watcher.setFuture(future);
}

std::tuple<QImage, quint64, double> TemplateMatchingModel::processImage(const QImage image,
                                                                         const QImage templateImage,
                                                                         const int method,
                                                                         const int thickness) {
    QElapsedTimer timer;
    timer.start();

    const cv::Mat source = QImageToMat(image);
    const cv::Mat templ = QImageToMat(templateImage);

    if (source.empty() || templ.empty()) {
        return {QImage(), timer.elapsed(), 0.0};
    }

    try {
        const TemplateMatchResult match = MatchTemplateImage(source, templ, method);
        const cv::Mat drawn = DrawTemplateMatch(source, match, cv::Scalar(0, 255, 0), thickness);
        return {MatToQImage(drawn), timer.elapsed(), match.score};
    } catch (const cv::Exception& exception) {
        LogOpenCvError("TemplateMatching", exception);
    } catch (const std::exception& exception) {
        LogStdError("TemplateMatching", exception);
    } catch (...) {
        LogUnknownError("TemplateMatching");
    }

    return {QImage(), timer.elapsed(), 0.0};
}
