#ifndef HOUGHCIRCLESMODEL_H
#define HOUGHCIRCLESMODEL_H

#include <tuple>

#include <QFutureWatcher>

#include <QtNodes/NodeDelegateModel>

#include "Nodes/Data/CirclesData.h"
#include "Nodes/Data/ImageData.h"

class QDoubleSpinBox;
class QSpinBox;
class QWidget;

class HoughCirclesModel final : public QtNodes::NodeDelegateModel {
    Q_OBJECT

public:
    HoughCirclesModel();

    ~HoughCirclesModel() override;

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
    static std::tuple<CirclesData, quint64> processImage(QImage image,
                                                         double dp,
                                                         double minDist,
                                                         double param1,
                                                         double param2,
                                                         int minRadius,
                                                         int maxRadius);

    QImage getImageToProcess() const;

private:
    QWidget* m_widget = nullptr;
    QDoubleSpinBox* m_dpSpinBox = nullptr;
    QDoubleSpinBox* m_minDistSpinBox = nullptr;
    QDoubleSpinBox* m_param1SpinBox = nullptr;
    QDoubleSpinBox* m_param2SpinBox = nullptr;
    QSpinBox* m_minRadiusSpinBox = nullptr;
    QSpinBox* m_maxRadiusSpinBox = nullptr;
    QSpinBox* m_timeSpinBox = nullptr;

    std::weak_ptr<ImageData> m_inImageData;
    std::shared_ptr<CirclesData> m_outCirclesData;
    QImage m_lastImageToProcess;
    QFutureWatcher<std::tuple<CirclesData, quint64>> m_watcher;

    double m_dp = 1.0;
    double m_minDist = 20.0;
    double m_param1 = 100.0;
    double m_param2 = 30.0;
    int m_minRadius = 0;
    int m_maxRadius = 0;
};

#endif // HOUGHCIRCLESMODEL_H
