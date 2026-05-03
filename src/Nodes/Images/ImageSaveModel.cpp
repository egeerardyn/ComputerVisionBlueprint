#include "ImageSaveModel.h"

#include "Nodes/NodeHelpInfo.h"
#include "Nodes/Images/ImageSaveUtils.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QVBoxLayout>

namespace {
const NodeHelpRegistration kImageSaveModelHelp(QStringLiteral("Save Image"),
                                                makeNodeHelp(QStringLiteral("Saves images to disk in selectable formats such as PNG, TIFF, BMP, JPEG, and OpenEXR."),
                                                             QStringLiteral("https://doc.qt.io/qt-6/qimage.html#save")));

QString SuggestedFilter() {
    return QStringLiteral("Images (*.png *.bmp *.jpg *.jpeg *.tiff *.tif *.exr *.ppm *.pgm)");
}

QByteArray FormatForComboText(const QString& value) {
    const QString upper = value.toUpper();
    if (upper == "JPG") {
        return "JPEG";
    }
    if (upper == "TIF") {
        return "TIFF";
    }
    return upper.toUtf8();
}
} // namespace

ImageSaveModel::ImageSaveModel() {
}

ImageSaveModel::~ImageSaveModel() {
}

QString ImageSaveModel::caption() const {
    return "Save Image";
}

QString ImageSaveModel::name() const {
    return "Save Image";
}

unsigned ImageSaveModel::nPorts(const QtNodes::PortType portType) const {
    switch (portType) {
        case QtNodes::PortType::In:
            return 2;
        case QtNodes::PortType::Out:
            return 2;
        default:
            return 0;
    }
}

QtNodes::NodeDataType ImageSaveModel::dataType(const QtNodes::PortType portType,
                                               const QtNodes::PortIndex portIndex) const {
    if (portType == QtNodes::PortType::In) {
        return portIndex == 0 ? ImageData().type() : FileData().type();
    }

    return portIndex == 0 ? ImageData().type() : FileData().type();
}

void ImageSaveModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, const QtNodes::PortIndex portIndex) {
    switch (portIndex) {
        case 0:
            m_inImageData = std::dynamic_pointer_cast<ImageData>(nodeData);
            break;
        case 1: {
            m_inFileData = std::dynamic_pointer_cast<FileData>(nodeData);
            const auto lock = m_inFileData.lock();
            if (lock && m_pathEdit) {
                QSignalBlocker blocker(m_pathEdit);
                m_pathEdit->setText(lock->fileName());
            }
            break;
        }
        default:
            break;
    }

    if (const auto imageLock = m_inImageData.lock()) {
        m_outImageData = std::make_shared<ImageData>(imageLock->image());
    } else {
        m_outImageData.reset();
    }
    emit dataUpdated(0);

    if (m_autoSaveCheck && m_autoSaveCheck->isChecked()) {
        saveCurrentImage();
    }
}

std::shared_ptr<QtNodes::NodeData> ImageSaveModel::outData(const QtNodes::PortIndex port) {
    switch (port) {
        case 0:
            return m_outImageData;
        case 1:
            return m_outFileData;
        default:
            return nullptr;
    }
}

QWidget* ImageSaveModel::embeddedWidget() {
    if (!m_widget) {
        m_widget = new QWidget();
        auto* rootLayout = new QVBoxLayout(m_widget);
        rootLayout->setContentsMargins(6, 6, 6, 6);

        auto* pathLayout = new QHBoxLayout();
        m_pathEdit = new QLineEdit(m_widget);
        m_browseButton = new QPushButton("...", m_widget);
        m_browseButton->setMaximumWidth(36);
        pathLayout->addWidget(m_pathEdit);
        pathLayout->addWidget(m_browseButton);

        auto* formLayout = new QFormLayout();
        formLayout->addRow("Path", pathLayout);

        m_formatCombo = new QComboBox(m_widget);
        m_formatCombo->addItems({"PNG", "BMP", "TIFF", "JPG", "EXR", "PPM", "PGM"});
        formLayout->addRow("Format", m_formatCombo);

        m_qualitySpin = new QSpinBox(m_widget);
        m_qualitySpin->setRange(-1, 100);
        m_qualitySpin->setValue(-1);
        formLayout->addRow("Quality", m_qualitySpin);

        m_autoSaveCheck = new QCheckBox("Auto save on input", m_widget);
        formLayout->addRow(m_autoSaveCheck);

        m_savedCheck = new QCheckBox("Saved", m_widget);
        m_savedCheck->setEnabled(false);
        formLayout->addRow(m_savedCheck);

        m_saveButton = new QPushButton("Save now", m_widget);
        formLayout->addRow(m_saveButton);

        rootLayout->addLayout(formLayout);

        connect(m_browseButton, &QPushButton::clicked, this, [this]() {
            const QString path = QFileDialog::getSaveFileName(nullptr,
                                                              tr("Save Image"),
                                                              QDir::homePath(),
                                                              SuggestedFilter());
            if (!path.isEmpty()) {
                pathEdited(path);
            }
        });
        connect(m_pathEdit, &QLineEdit::textChanged, this, &ImageSaveModel::pathEdited);
        connect(m_saveButton, &QPushButton::clicked, this, &ImageSaveModel::saveClicked);
    }

    return m_widget;
}

void ImageSaveModel::saveClicked() {
    saveCurrentImage();
}

void ImageSaveModel::pathEdited(const QString& path) {
    if (m_pathEdit && m_pathEdit->text() != path) {
        QSignalBlocker blocker(m_pathEdit);
        m_pathEdit->setText(path);
    }

    if (m_savedCheck) {
        m_savedCheck->setChecked(false);
    }
}

bool ImageSaveModel::saveCurrentImage() {
    const auto imageLock = m_inImageData.lock();
    if (!imageLock || imageLock->isNull()) {
        if (m_savedCheck) {
            m_savedCheck->setChecked(false);
        }
        return false;
    }

    QString path;
    if (m_pathEdit && !m_pathEdit->text().isEmpty()) {
        path = m_pathEdit->text();
    } else if (const auto fileLock = m_inFileData.lock()) {
        path = fileLock->fileName();
        if (m_pathEdit) {
            QSignalBlocker blocker(m_pathEdit);
            m_pathEdit->setText(path);
        }
    }

    if (path.isEmpty()) {
        if (m_savedCheck) {
            m_savedCheck->setChecked(false);
        }
        return false;
    }

    QByteArray format = "PNG";
    if (m_formatCombo) {
        format = FormatForComboText(m_formatCombo->currentText());
    }

    if (!QFileInfo(path).suffix().isEmpty()) {
        const QString ext = QFileInfo(path).suffix();
        format = FormatForComboText(ext);
    }

    const int quality = m_qualitySpin ? m_qualitySpin->value() : -1;
    const bool ok = SaveImageToFile(imageLock->image(), path, format, quality);

    if (m_savedCheck) {
        m_savedCheck->setChecked(ok);
    }

    if (ok) {
        m_outFileData = std::make_shared<FileData>(path);
        emit dataUpdated(1);
    }

    return ok;
}
