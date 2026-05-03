//
// Created by pablo on 4/21/24.
//

#include "DrawCirclesModel.h"

#include <QElapsedTimer>
#include <QPainter>
#include <QPen>
#include <QSignalBlocker>
#include <QtConcurrent/QtConcurrent>

#include "ui_DrawCirclesForm.h"

DrawCirclesModel::DrawCirclesModel() {
    connect(&m_watcher, &QFutureWatcher<QPair<QImage, quint64>>::finished, this, &DrawCirclesModel::processFinished);
}

DrawCirclesModel::~DrawCirclesModel() {
}

QString DrawCirclesModel::caption() const {
    return QStringLiteral("Draw Circles");
}

QString DrawCirclesModel::name() const {
    return QStringLiteral("Draw Circles");
}

unsigned DrawCirclesModel::nPorts(const QtNodes::PortType portType) const {
    switch (portType) {
        case QtNodes::PortType::In:
            return 4;
        case QtNodes::PortType::Out:
            return 1;
        default:
            return 0;
    }
}

QtNodes::NodeDataType DrawCirclesModel::dataType(const QtNodes::PortType portType, const QtNodes::PortIndex portIndex) const {
    switch (portType) {
        case QtNodes::PortType::In:
            switch (portIndex) {
                case 0:
                    return ImageData().type();
                case 1:
                    return CirclesData().type();
                case 2:
                    return VariantData(QColor()).typeIn();
                case 3:
                    return VariantData(0).typeIn();
                default:
                    return VariantData().type();
            }
        case QtNodes::PortType::Out:
            return ImageData().type();
        default:
            return VariantData().type();
    }
}

void DrawCirclesModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, const QtNodes::PortIndex portIndex) {
    switch (portIndex) {
        case 0:
            m_inImageData = std::dynamic_pointer_cast<ImageData>(nodeData);
            if (m_inImageData.expired()) {
                m_lastPixmapToProcess = QImage();
                m_outImageData.reset();
                emit dataUpdated(0);
                return;
            }
            break;
        case 1:
            m_inCirclesData = std::dynamic_pointer_cast<CirclesData>(nodeData);
            if (m_inCirclesData.expired()) {
                m_outImageData.reset();
                emit dataUpdated(0);
                return;
            }
            break;
        case 2:
            m_inColor = std::dynamic_pointer_cast<VariantData>(nodeData);
            break;
        case 3:
            m_inThickness = std::dynamic_pointer_cast<VariantData>(nodeData);
            break;
        default:
            break;
    }
    updateFromInputPort();
}

std::shared_ptr<QtNodes::NodeData> DrawCirclesModel::outData(const QtNodes::PortIndex port) {
    Q_UNUSED(port)
    return m_outImageData;
}

QWidget* DrawCirclesModel::embeddedWidget() {
    if (!m_widget) {
        m_ui.reset(new Ui::DrawCirclesForm);
        m_widget = new QWidget();
        m_ui->setupUi(m_widget);

        connect(m_ui->sb_r, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
            m_color.setRed(value);
            m_lastPixmapToProcess = getPixmapToProcess();
            requestProcess();
        });
        connect(m_ui->sb_g, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
            m_color.setGreen(value);
            m_lastPixmapToProcess = getPixmapToProcess();
            requestProcess();
        });
        connect(m_ui->sb_b, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
            m_color.setBlue(value);
            m_lastPixmapToProcess = getPixmapToProcess();
            requestProcess();
        });
        connect(m_ui->sb_thickness, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
            m_thickness = value;
            m_lastPixmapToProcess = getPixmapToProcess();
            requestProcess();
        });

        m_color = QColor(m_ui->sb_r->value(), m_ui->sb_g->value(), m_ui->sb_b->value());
        m_thickness = m_ui->sb_thickness->value();
        updateFromInputPort();
    }
    return m_widget;
}

QPair<QImage, quint64> DrawCirclesModel::processImage(QImage image, const Circles& circles, const QColor& color,
                                                      const int thickness) {
    QElapsedTimer timer;
    timer.start();
    QPainter painter(&image);

    painter.setPen(QPen(color, thickness));
    for (const Circle& circle : circles) {
        painter.drawEllipse(circle.center, circle.radius, circle.radius);
    }
    painter.end();

    return {image, static_cast<quint64>(timer.elapsed())};
}

void DrawCirclesModel::updateFromInputPort() {
    m_lastPixmapToProcess = getPixmapToProcess();

    if (m_ui) {
        const auto lockColor = m_inColor.lock();
        if (lockColor && lockColor->metaType() == QMetaType::QColor) {
            QSignalBlocker blockR(m_ui->sb_r);
            QSignalBlocker blockG(m_ui->sb_g);
            QSignalBlocker blockB(m_ui->sb_b);
            m_color = lockColor->variant().value<QColor>();
            m_ui->sb_r->setEnabled(false);
            m_ui->sb_g->setEnabled(false);
            m_ui->sb_b->setEnabled(false);
            m_ui->sb_r->setValue(m_color.red());
            m_ui->sb_g->setValue(m_color.green());
            m_ui->sb_b->setValue(m_color.blue());
        } else {
            m_ui->sb_r->setEnabled(true);
            m_ui->sb_g->setEnabled(true);
            m_ui->sb_b->setEnabled(true);
        }

        const auto lockThickness = m_inThickness.lock();
        if (lockThickness && lockThickness->metaType() == QMetaType::Int) {
            QSignalBlocker blockThickness(m_ui->sb_thickness);
            m_thickness = lockThickness->variant().toInt();
            m_ui->sb_thickness->setEnabled(false);
            m_ui->sb_thickness->setValue(m_thickness);
        } else {
            m_ui->sb_thickness->setEnabled(true);
        }
    } else {
        if (const auto lockColor = m_inColor.lock(); lockColor && lockColor->metaType() == QMetaType::QColor) {
            m_color = lockColor->variant().value<QColor>();
        }
        if (const auto lockThickness = m_inThickness.lock(); lockThickness && lockThickness->metaType() == QMetaType::Int) {
            m_thickness = lockThickness->variant().toInt();
        }
    }

    requestProcess();
}

QImage DrawCirclesModel::getPixmapToProcess() const {
    const auto lock = m_inImageData.lock();
    if (lock) {
        return lock->image();
    }
    return QImage();
}

void DrawCirclesModel::processFinished() {
    const auto [image, elapsed] = m_watcher.result();
    m_outImageData = std::make_shared<ImageData>(image);
    if (m_ui) {
        m_ui->sb_time->setValue(elapsed);
    }

    emit dataUpdated(0);
    m_processing = false;
    requestProcess();
}

void DrawCirclesModel::requestProcess() {
    if (m_inImageData.expired() || m_inCirclesData.expired() || m_processing || m_lastPixmapToProcess.isNull()) {
        return;
    }

    m_processing = true;
    const auto future = QtConcurrent::run(&DrawCirclesModel::processImage, m_lastPixmapToProcess,
                                          m_inCirclesData.lock()->circles(), m_color, m_thickness);
    m_lastPixmapToProcess = QImage();
    m_watcher.setFuture(future);
}
