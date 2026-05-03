#ifndef BILATERALFILTERMODEL_H
#define BILATERALFILTERMODEL_H

#include <QFutureWatcher>

#include <opencv2/opencv.hpp>

#include <QtNodes/NodeDelegateModel>

#include "Nodes/Data/ImageData.h"

namespace Ui {
    class BilateralFilterForm;
}

class BilateralFilterModel final : public QtNodes::NodeDelegateModel {
    Q_OBJECT

public:
    BilateralFilterModel();

    ~BilateralFilterModel() override;

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
    static std::tuple<QImage, quint64> processImage(QImage image, int diameter, double sigmaColor, double sigmaSpace,
                                                    int borderType);

    QImage getImageToProcess() const;

private:
    QWidget* m_widget = nullptr;
    std::unique_ptr<Ui::BilateralFilterForm> m_ui;
    std::weak_ptr<ImageData> m_inImageData;
    std::shared_ptr<ImageData> m_outImageData;
    QImage m_lastImageToProcess;
    int m_diameter = 9;
    double m_sigmaColor = 75.0;
    double m_sigmaSpace = 75.0;
    int m_borderType = cv::BORDER_DEFAULT;
    QFutureWatcher<std::tuple<QImage, quint64>> m_watcher;
};

#endif //BILATERALFILTERMODEL_H
