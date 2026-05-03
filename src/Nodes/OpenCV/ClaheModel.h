#ifndef CLAHEMODEL_H
#define CLAHEMODEL_H

#include <tuple>

#include <QFutureWatcher>

#include <QtNodes/NodeDelegateModel>

#include "Nodes/Data/ImageData.h"

class QDoubleSpinBox;
class QSpinBox;
class QWidget;

class ClaheModel final : public QtNodes::NodeDelegateModel {
    Q_OBJECT

public:
    ClaheModel();

    ~ClaheModel() override;

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
    static std::tuple<QImage, quint64> processImage(QImage image, double clipLimit, int tileSize);

    QImage getImageToProcess() const;

private:
    QWidget* m_widget = nullptr;
    QDoubleSpinBox* m_clipLimitSpinBox = nullptr;
    QSpinBox* m_tileSizeSpinBox = nullptr;
    QSpinBox* m_timeSpinBox = nullptr;

    std::weak_ptr<ImageData> m_inImageData;
    std::shared_ptr<ImageData> m_outImageData;
    QImage m_lastImageToProcess;
    QFutureWatcher<std::tuple<QImage, quint64>> m_watcher;

    double m_clipLimit = 2.0;
    int m_tileSize = 8;
};

#endif // CLAHEMODEL_H
