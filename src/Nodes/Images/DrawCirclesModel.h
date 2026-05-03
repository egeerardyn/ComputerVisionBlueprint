//
// Created by pablo on 4/21/24.
//

#ifndef DRAWCIRCLESMODEL_H
#define DRAWCIRCLESMODEL_H

#include <QtNodes/NodeDelegateModel>
#include <QFutureWatcher>

#include "Nodes/Data/CirclesData.h"
#include "Nodes/Data/ImageData.h"
#include "Nodes/Data/VariantData.h"

namespace Ui {
    class DrawCirclesForm;
}

class DrawCirclesModel final : public QtNodes::NodeDelegateModel {
    Q_OBJECT

public:
    DrawCirclesModel();

    ~DrawCirclesModel() override;

    QString caption() const override;

    QString name() const override;

    unsigned nPorts(QtNodes::PortType portType) const override;

    QtNodes::NodeDataType dataType(QtNodes::PortType portType, QtNodes::PortIndex portIndex) const override;

    void setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex portIndex) override;

    std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex port) override;

    QWidget* embeddedWidget() override;

private:
    static QPair<QImage, quint64> processImage(QImage image, const Circles& circles, const QColor& color, int thickness);

    void updateFromInputPort();

    QImage getPixmapToProcess() const;

private slots:
    void processFinished();

    void requestProcess();

private:
    QWidget* m_widget = nullptr;
    QScopedPointer<Ui::DrawCirclesForm> m_ui;
    QFutureWatcher<QPair<QImage, quint64>> m_watcher;
    bool m_processing = false;

    QImage m_lastPixmapToProcess;
    std::weak_ptr<ImageData> m_inImageData;
    std::weak_ptr<CirclesData> m_inCirclesData;
    QColor m_color = Qt::black;
    std::weak_ptr<VariantData> m_inColor;
    int m_thickness = 20;
    std::weak_ptr<VariantData> m_inThickness;

    std::shared_ptr<ImageData> m_outImageData;
};

#endif //DRAWCIRCLESMODEL_H
