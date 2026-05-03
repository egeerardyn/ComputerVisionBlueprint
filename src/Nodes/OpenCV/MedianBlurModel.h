#ifndef MEDIANBLURMODEL_H
#define MEDIANBLURMODEL_H

#include <QFutureWatcher>

#include <QtNodes/NodeDelegateModel>

#include "Nodes/Data/ImageData.h"

namespace Ui {
    class MedianBlurForm;
}

class MedianBlurModel final : public QtNodes::NodeDelegateModel {
    Q_OBJECT

public:
    MedianBlurModel();

    ~MedianBlurModel() override;

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
    static std::tuple<QImage, quint64> processImage(QImage image, int kernelSize);

    QImage getImageToProcess() const;

private:
    QWidget* m_widget = nullptr;
    std::unique_ptr<Ui::MedianBlurForm> m_ui;
    std::weak_ptr<ImageData> m_inImageData;
    std::shared_ptr<ImageData> m_outImageData;
    QImage m_lastImageToProcess;
    int m_kernelSize = 3;
    QFutureWatcher<std::tuple<QImage, quint64>> m_watcher;
};

#endif //MEDIANBLURMODEL_H
