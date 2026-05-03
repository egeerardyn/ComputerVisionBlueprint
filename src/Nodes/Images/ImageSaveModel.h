#ifndef IMAGESAVEMODEL_H
#define IMAGESAVEMODEL_H

#include <QtNodes/NodeDelegateModel>

#include "Nodes/Data/FileData.h"
#include "Nodes/Data/ImageData.h"

class QCheckBox;
class QComboBox;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QWidget;

class ImageSaveModel final : public QtNodes::NodeDelegateModel {
    Q_OBJECT

public:
    ImageSaveModel();

    ~ImageSaveModel() override;

    QString caption() const override;

    QString name() const override;

    unsigned nPorts(QtNodes::PortType portType) const override;

    QtNodes::NodeDataType dataType(QtNodes::PortType portType, QtNodes::PortIndex portIndex) const override;

    void setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex portIndex) override;

    std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex port) override;

    QWidget* embeddedWidget() override;

private slots:
    void saveClicked();

    void pathEdited(const QString& path);

private:
    bool saveCurrentImage();

private:
    QWidget* m_widget = nullptr;
    QLineEdit* m_pathEdit = nullptr;
    QComboBox* m_formatCombo = nullptr;
    QSpinBox* m_qualitySpin = nullptr;
    QCheckBox* m_autoSaveCheck = nullptr;
    QCheckBox* m_savedCheck = nullptr;
    QPushButton* m_browseButton = nullptr;
    QPushButton* m_saveButton = nullptr;

    std::weak_ptr<ImageData> m_inImageData;
    std::weak_ptr<FileData> m_inFileData;

    std::shared_ptr<ImageData> m_outImageData;
    std::shared_ptr<FileData> m_outFileData;
};

#endif // IMAGESAVEMODEL_H
