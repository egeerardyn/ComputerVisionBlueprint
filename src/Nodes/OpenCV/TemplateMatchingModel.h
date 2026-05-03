#ifndef TEMPLATEMATCHINGMODEL_H
#define TEMPLATEMATCHINGMODEL_H

#include <tuple>

#include <QFutureWatcher>

#include <QtNodes/NodeDelegateModel>

#include <opencv2/imgproc.hpp>

#include "Nodes/Data/ImageData.h"
#include "Nodes/OpenCV/TemplateMatchingUtils.h"

class QComboBox;
class QSpinBox;
class QDoubleSpinBox;
class QWidget;

class TemplateMatchingModel final : public QtNodes::NodeDelegateModel {
    Q_OBJECT

public:
    TemplateMatchingModel();

    ~TemplateMatchingModel() override;

    QString caption() const override;

    QString name() const override;

    unsigned nPorts(QtNodes::PortType portType) const override;

    QtNodes::NodeDataType dataType(QtNodes::PortType portType, QtNodes::PortIndex portIndex) const override;

    void setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex portIndex) override;

    std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex port) override;

    QWidget* embeddedWidget() override;

    bool portCaptionVisible(QtNodes::PortType, QtNodes::PortIndex) const override;

    QString portCaption(QtNodes::PortType portType, QtNodes::PortIndex portIndex) const override;

private slots:
    void processFinished();

    void requestProcess();

private:
    static std::tuple<QImage, quint64, double> processImage(QImage image,
                                                             QImage templateImage,
                                                             int method,
                                                             int thickness);

private:
    QWidget* m_widget = nullptr;
    QComboBox* m_methodCombo = nullptr;
    QSpinBox* m_thicknessSpinBox = nullptr;
    QDoubleSpinBox* m_scoreSpinBox = nullptr;
    QSpinBox* m_timeSpinBox = nullptr;

    std::weak_ptr<ImageData> m_inImageData;
    std::weak_ptr<ImageData> m_inTemplateData;
    std::shared_ptr<ImageData> m_outImageData;

    QImage m_lastImageToProcess;
    QImage m_lastTemplateToProcess;

    QFutureWatcher<std::tuple<QImage, quint64, double>> m_watcher;

    int m_method = cv::TM_CCOEFF_NORMED;
    int m_thickness = 2;
};

#endif // TEMPLATEMATCHINGMODEL_H
