#ifndef MORPHOLOGYMODELBASE_H
#define MORPHOLOGYMODELBASE_H

#include <memory>
#include <tuple>
#include <vector>

#include <QFutureWatcher>

#include <opencv2/opencv.hpp>

#include <QtNodes/NodeDelegateModel>

#include "Nodes/Data/ImageData.h"

class QComboBox;
class QSpinBox;
class QWidget;

class MorphologyModelBase : public QtNodes::NodeDelegateModel {
    Q_OBJECT

public:
    struct OperationOption {
        QString label;
        int operation;
    };

    MorphologyModelBase(QString title, int defaultOperation, std::vector<OperationOption> operationOptions = {});

    ~MorphologyModelBase() override;

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
    static std::tuple<QImage, quint64> processImage(QImage image, QSize kernelSize, int kernelShape, int iterations,
                                                    int operation, bool useMorphologyEx);

    void updatePendingImage();

    QImage getImageToProcess() const;

    void createWidget();

private:
    QString m_title;
    QWidget* m_widget = nullptr;
    QComboBox* m_shapeCombo = nullptr;
    QComboBox* m_operationCombo = nullptr;
    QSpinBox* m_kernelWidthSpinBox = nullptr;
    QSpinBox* m_kernelHeightSpinBox = nullptr;
    QSpinBox* m_iterationsSpinBox = nullptr;
    QSpinBox* m_timeSpinBox = nullptr;

    std::vector<OperationOption> m_operationOptions;
    std::weak_ptr<ImageData> m_inImageData;
    std::shared_ptr<ImageData> m_outImageData;
    QImage m_lastImageToProcess;
    QSize m_kernelSize = QSize(3, 3);
    int m_kernelShape = cv::MORPH_RECT;
    int m_iterations = 1;
    int m_operation = cv::MORPH_ERODE;
    bool m_useMorphologyEx = false;
    QFutureWatcher<std::tuple<QImage, quint64>> m_watcher;
};

#endif //MORPHOLOGYMODELBASE_H
