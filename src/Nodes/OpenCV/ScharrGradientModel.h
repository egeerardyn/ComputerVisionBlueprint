#ifndef SCHARRGRADIENTMODEL_H
#define SCHARRGRADIENTMODEL_H

#include <tuple>

#include <QFutureWatcher>

#include <QtNodes/NodeDelegateModel>

#include "Nodes/Data/ImageData.h"
#include "Nodes/OpenCV/GradientUtils.h"

class QComboBox;
class QDoubleSpinBox;
class QSpinBox;
class QWidget;

class ScharrGradientModel final : public QtNodes::NodeDelegateModel {
    Q_OBJECT

public:
    ScharrGradientModel();

    ~ScharrGradientModel() override;

    QString caption() const override;

    QString name() const override;

    unsigned nPorts(QtNodes::PortType portType) const override;

    QtNodes::NodeDataType dataType(QtNodes::PortType portType, QtNodes::PortIndex portIndex) const override;

    void setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex portIndex) override;

    std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex port) override;

    QWidget* embeddedWidget() override;

private slots:
    void processFinished();

    void requestProcess();

private:
    static std::tuple<QImage, quint64> processImage(QImage image,
                                                     double scale,
                                                     double delta,
                                                     GradientOutputMode mode);

    QImage getImageToProcess() const;

private:
    QWidget* m_widget = nullptr;
    QComboBox* m_modeCombo = nullptr;
    QDoubleSpinBox* m_scaleSpinBox = nullptr;
    QDoubleSpinBox* m_deltaSpinBox = nullptr;
    QSpinBox* m_timeSpinBox = nullptr;

    std::weak_ptr<ImageData> m_inImageData;
    std::shared_ptr<ImageData> m_outImageData;
    QImage m_lastImageToProcess;

    QFutureWatcher<std::tuple<QImage, quint64>> m_watcher;
    double m_scale = 1.0;
    double m_delta = 0.0;
    GradientOutputMode m_mode = GradientOutputMode::Magnitude;
};

#endif // SCHARRGRADIENTMODEL_H
