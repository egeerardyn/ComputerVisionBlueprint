#ifndef HISTOGRAMMODEL_H
#define HISTOGRAMMODEL_H

#include <tuple>

#include <QFutureWatcher>

#include <QtNodes/NodeDelegateModel>

#include "Nodes/Data/ImageData.h"

class QSpinBox;
class QWidget;

class HistogramModel final : public QtNodes::NodeDelegateModel {
    Q_OBJECT

public:
    HistogramModel();

    ~HistogramModel() override;

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
    static std::tuple<QImage, quint64> processImage(QImage image, int bins, int width, int height);

    QImage getImageToProcess() const;

private:
    QWidget* m_widget = nullptr;
    QSpinBox* m_binsSpinBox = nullptr;
    QSpinBox* m_widthSpinBox = nullptr;
    QSpinBox* m_heightSpinBox = nullptr;
    QSpinBox* m_timeSpinBox = nullptr;

    std::weak_ptr<ImageData> m_inImageData;
    std::shared_ptr<ImageData> m_outImageData;
    QImage m_lastImageToProcess;
    QFutureWatcher<std::tuple<QImage, quint64>> m_watcher;

    int m_bins = 64;
    int m_width = 320;
    int m_height = 220;
};

#endif // HISTOGRAMMODEL_H
