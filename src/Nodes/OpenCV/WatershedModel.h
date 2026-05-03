#ifndef WATERSHEDMODEL_H
#define WATERSHEDMODEL_H

#include <tuple>

#include <QFutureWatcher>

#include <QtNodes/NodeDelegateModel>

#include "Nodes/Data/ImageData.h"
#include "Nodes/Data/MarkersData.h"

class QDoubleSpinBox;
class QSpinBox;
class QWidget;

class WatershedModel final : public QtNodes::NodeDelegateModel {
    Q_OBJECT

public:
    WatershedModel();

    ~WatershedModel() override;

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
    static std::tuple<QImage, cv::Mat, int, quint64> processImage(QImage image, cv::Mat markers, double overlayAlpha);

    void updatePendingInputs();

private:
    QWidget* m_widget = nullptr;
    QDoubleSpinBox* m_overlayAlphaSpinBox = nullptr;
    QDoubleSpinBox* m_timeSpinBox = nullptr;
    QSpinBox* m_regionsSpinBox = nullptr;

    std::weak_ptr<ImageData> m_inImageData;
    std::weak_ptr<MarkersData> m_inMarkersData;
    std::shared_ptr<ImageData> m_outOverlayData;
    std::shared_ptr<MarkersData> m_outLabelsData;
    std::shared_ptr<ImageData> m_outLabelsPreviewData;
    QImage m_lastImageToProcess;
    cv::Mat m_lastMarkersToProcess;
    double m_overlayAlpha = 0.35;
    QFutureWatcher<std::tuple<QImage, cv::Mat, int, quint64>> m_watcher;
};

#endif //WATERSHEDMODEL_H
