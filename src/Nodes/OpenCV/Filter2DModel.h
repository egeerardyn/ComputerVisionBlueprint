#ifndef FILTER2DMODEL_H
#define FILTER2DMODEL_H

#include <array>

#include <QFutureWatcher>

#include <QtNodes/NodeDelegateModel>

#include "Nodes/Data/ImageData.h"

class QComboBox;
class QDoubleSpinBox;
class QWidget;

class Filter2DModel final : public QtNodes::NodeDelegateModel {
    Q_OBJECT

public:
    Filter2DModel();

    ~Filter2DModel() override;

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
    static std::tuple<QImage, quint64> processImage(QImage image, std::array<double, 9> kernelValues, double delta);

    void applyPreset(int presetIndex);

    void syncKernelFromUi();

    QImage getImageToProcess() const;

private:
    QWidget* m_widget = nullptr;
    QComboBox* m_presetCombo = nullptr;
    QDoubleSpinBox* m_deltaSpinBox = nullptr;
    QDoubleSpinBox* m_timeSpinBox = nullptr;
    std::array<QDoubleSpinBox*, 9> m_kernelSpinBoxes{};

    std::weak_ptr<ImageData> m_inImageData;
    std::shared_ptr<ImageData> m_outImageData;
    QImage m_lastImageToProcess;
    std::array<double, 9> m_kernelValues{0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0};
    double m_delta = 0.0;
    QFutureWatcher<std::tuple<QImage, quint64>> m_watcher;
};

#endif //FILTER2DMODEL_H
