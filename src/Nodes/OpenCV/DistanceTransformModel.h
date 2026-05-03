#ifndef DISTANCETRANSFORMMODEL_H
#define DISTANCETRANSFORMMODEL_H

#include <tuple>

#include <QFutureWatcher>

#include <opencv2/imgproc.hpp>

#include <QtNodes/NodeDelegateModel>

#include "Nodes/Data/ImageData.h"
#include "Nodes/Data/MarkersData.h"

class QComboBox;
class QDoubleSpinBox;
class QWidget;

class DistanceTransformModel final : public QtNodes::NodeDelegateModel {
    Q_OBJECT

public:
    DistanceTransformModel();

    ~DistanceTransformModel() override;

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
    static std::tuple<QImage, cv::Mat, quint64> processImage(QImage image, int distanceType, int maskSize,
                                                             double markerThreshold);

    QImage getImageToProcess() const;

private:
    QWidget* m_widget = nullptr;
    QComboBox* m_distanceTypeCombo = nullptr;
    QComboBox* m_maskSizeCombo = nullptr;
    QDoubleSpinBox* m_markerThresholdSpinBox = nullptr;
    QDoubleSpinBox* m_timeSpinBox = nullptr;

    std::weak_ptr<ImageData> m_inImageData;
    std::shared_ptr<ImageData> m_outDistanceImageData;
    std::shared_ptr<MarkersData> m_outMarkersData;
    std::shared_ptr<ImageData> m_outMarkersPreviewData;
    QImage m_lastImageToProcess;
    int m_distanceType = cv::DIST_L2;
    int m_maskSize = cv::DIST_MASK_3;
    double m_markerThreshold = 0.4;
    QFutureWatcher<std::tuple<QImage, cv::Mat, quint64>> m_watcher;
};

#endif //DISTANCETRANSFORMMODEL_H
