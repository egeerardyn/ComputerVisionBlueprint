//
// Created by pablo on 2/24/24.
//

// You may need to build the project (run Qt uic code generator) to get "ui_MainWindow.h" resolved

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QActionGroup>
#include <QDockWidget>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QMenu>
#include <QStatusBar>

#include <QtNodes/DataFlowGraphicsScene>
#include <QtNodes/DataFlowGraphModel>
#include <QtNodes/NodeDelegateModel>
#include "Nodes/NodesInclude.h"
#include "Nodes/NodeHelpInfo.h"
#include "UndoCommands.hpp"

#include <QtNodes/GraphicsView>

#include "Widgets/HelpWidget.h"
#include "Widgets/ThemeControlsWidget.h"
#include "Widgets/ThemeManager.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    m_themeManager = new ThemeManager(this);
    createThemeUi();
    createHelpUi();
    connect(m_themeManager, &ThemeManager::themeChanged, this, [this]() {
        m_themeManager->apply();
        updateThemeActions();
        statusBar()->showMessage(QString("Applied %1 theme").arg(ThemeManager::presetLabel(m_themeManager->configuration().preset)), 2500);
    });
    m_themeManager->apply();
    updateThemeActions();

    const auto registers = registerDataModels();
    ui->tw_nodes->fillTreeWidget(registers);

    m_model = new QtNodes::DataFlowGraphModel(registers);
    m_scene = new QtNodes::DataFlowGraphicsScene(*m_model);
    m_model->setParent(m_scene);

    ui->nodes_graphicsView->setMapGroupNames(ui->tw_nodes->getMapGroupNames());
    ui->nodes_graphicsView->setDataFlowScene(m_scene);
    ui->nodes_graphicsView->setScene(m_scene);
    connect(m_scene, &QGraphicsScene::selectionChanged, this, &MainWindow::updateHelpForSelection);
    connect(ui->tw_nodes, &DragableTreeOfNodes::nodeSelected, this, &MainWindow::showNodePaletteHelp);
    connect(ui->tw_nodes, &DragableTreeOfNodes::nodeSelectionCleared, this, &MainWindow::clearNodePaletteHelp);

    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::onActionSaveTriggered);
    connect(ui->actionLoad, &QAction::triggered, this, &MainWindow::onActionLoadTriggered);

    updateHelpForSelection();

    statusBar()->showMessage("Ready", 2000);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::mouseMoveEvent(QMouseEvent* event) {
    QMainWindow::mouseMoveEvent(event);
    // print the mouse position
    // qDebug() << event->pos();
}

void MainWindow::onActionSaveTriggered() {
    m_scene->save();
}

void MainWindow::onActionLoadTriggered() {
    m_scene->load();
}

std::shared_ptr<QtNodes::NodeDelegateModelRegistry> MainWindow::registerDataModels() {
    auto ret = std::make_shared<QtNodes::NodeDelegateModelRegistry>();
    ret->registerModel<PiModel>("Constants");

    ret->registerModel<NumberSourceDataModel>("Sources");

    ret->registerModel<NumberDisplayDataModel>("Displays");

    ret->registerModel<AdditionModel>("Operators");
    ret->registerModel<SubtractionModel>("Operators");
    ret->registerModel<MultiplicationModel>("Operators");
    ret->registerModel<DivisionModel>("Operators");

    ret->registerModel<ImageLoaderModel>("Images");
    ret->registerModel<ImageShowModel>("Images");
    ret->registerModel<ImageInfoModel>("Images");
    ret->registerModel<DrawCirclesModel>("Images");
    ret->registerModel<DrawLinesModel>("Images");
    ret->registerModel<DrawRectsModel>("Images");
    ret->registerModel<ConvertImageToModel>("Images");
    ret->registerModel<ScaleImageModel>("Images");
    ret->registerModel<CutImageModel>("Images");

    ret->registerModel<ColorCVModel>("OpenCV");
    ret->registerModel<BlurModel>("OpenCV");
    ret->registerModel<CannyModel>("OpenCV");
    ret->registerModel<GaussianBlurModel>("OpenCV");
    ret->registerModel<MedianBlurModel>("OpenCV");
    ret->registerModel<BilateralFilterModel>("OpenCV");
    ret->registerModel<BoxFilterModel>("OpenCV");
    ret->registerModel<SqrBoxFilterModel>("OpenCV");
    ret->registerModel<Filter2DModel>("OpenCV");
    ret->registerModel<ErodeModel>("OpenCV");
    ret->registerModel<DilateModel>("OpenCV");
    ret->registerModel<MorphologyExModel>("OpenCV");
    ret->registerModel<DistanceTransformModel>("OpenCV");
    ret->registerModel<WatershedModel>("OpenCV");
    ret->registerModel<ConvertDepthChannelsModel>("OpenCV");
    ret->registerModel<HoughLinesPModel>("OpenCV");
    ret->registerModel<EqualizeHistModel>("OpenCV");
    ret->registerModel<PyrDown>("OpenCV");

    ret->registerModel<CascadeClassifier>("CascadeClassifier");
    ret->registerModel<DetectMultiScaleModel>("CascadeClassifier");

    ret->registerModel<SizeVarModel>("Variables");
    ret->registerModel<FileVarModel>("Variables");
    ret->registerModel<CircleModel>("Variables");
    ret->registerModel<CircleVarModel>("Variables");
    ret->registerModel<RectVarModel>("Variables");
    ret->registerModel<RectModel>("Variables");
    ret->registerModel<FileFromUrlModel>("Variables");

    ret->registerModel<CameraModel>("Video");
    ret->registerModel<CaptureModel>("Video");

    ret->registerModel<ScaleRects>("Data Operations");

    return ret;
}

void MainWindow::createThemeUi() {
    auto* viewMenu = ui->menubar->addMenu("&View");
    auto* themeMenu = ui->menubar->addMenu("&Theme");

    m_themeDock = new QDockWidget("Theme Controls", this);
    m_themeDock->setObjectName("ThemeControlsDock");
    m_themeControlsWidget = new ThemeControlsWidget(m_themeManager, m_themeDock);
    m_themeDock->setWidget(m_themeControlsWidget);
    addDockWidget(Qt::RightDockWidgetArea, m_themeDock);
    m_themeDock->hide();

    auto* actionGroup = new QActionGroup(this);
    actionGroup->setExclusive(true);

    m_darkThemeAction = themeMenu->addAction("Dark");
    m_darkThemeAction->setCheckable(true);
    actionGroup->addAction(m_darkThemeAction);
    connect(m_darkThemeAction, &QAction::triggered, this, [this]() {
        m_themeManager->setPreset(ThemePreset::Dark);
    });

    m_lightThemeAction = themeMenu->addAction("Light");
    m_lightThemeAction->setCheckable(true);
    actionGroup->addAction(m_lightThemeAction);
    connect(m_lightThemeAction, &QAction::triggered, this, [this]() {
        m_themeManager->setPreset(ThemePreset::Light);
    });

    m_customThemeAction = themeMenu->addAction("Custom");
    m_customThemeAction->setCheckable(true);
    actionGroup->addAction(m_customThemeAction);
    connect(m_customThemeAction, &QAction::triggered, this, [this]() {
        m_themeManager->setPreset(ThemePreset::Custom);
        m_themeDock->show();
        m_themeDock->raise();
    });

    themeMenu->addSeparator();
    auto* showThemeControlsAction = m_themeDock->toggleViewAction();
    showThemeControlsAction->setText("Theme Controls");
    themeMenu->addAction(showThemeControlsAction);
    viewMenu->addAction(showThemeControlsAction);
}

void MainWindow::createHelpUi() {
    auto* helpMenu = ui->menubar->addMenu("&Help");

    m_helpDock = new QDockWidget("Help", this);
    m_helpDock->setObjectName("HelpDock");
    m_helpWidget = new HelpWidget(m_helpDock);
    m_helpDock->setWidget(m_helpWidget);
    addDockWidget(Qt::RightDockWidgetArea, m_helpDock);
    m_helpDock->show();

    if (m_themeDock) {
        tabifyDockWidget(m_themeDock, m_helpDock);
    }

    auto* showHelpAction = m_helpDock->toggleViewAction();
    showHelpAction->setText("Help");
    helpMenu->addAction(showHelpAction);
}

void MainWindow::updateHelpForSelection() {
    if (!m_helpWidget || !m_scene || !m_model) {
        return;
    }

    const std::vector<QtNodes::NodeId> selectedNodeIds = m_scene->selectedNodes();
    if (selectedNodeIds.empty()) {
        m_helpWidget->showDefaultHelp();
        return;
    }

    if (selectedNodeIds.size() > 1) {
        m_helpWidget->showMultiSelectionHelp(static_cast<int>(selectedNodeIds.size()));
        return;
    }

    auto* delegateModel = m_model->delegateModel<QtNodes::NodeDelegateModel>(selectedNodeIds.front());
    if (!delegateModel) {
        m_helpWidget->showDefaultHelp();
        return;
    }

    const QString nodeTitle = delegateModel->caption().isEmpty() ? delegateModel->name() : delegateModel->caption();
    m_helpWidget->showNodeHelp(nodeTitle, nodeHelpInfoForModel(*delegateModel));
}

void MainWindow::showNodePaletteHelp(const QString& nodeName) {
    if (!m_helpWidget || nodeName.isEmpty()) {
        return;
    }

    m_helpWidget->showNodeHelp(nodeName, nodeHelpInfoForModelName(nodeName));
}

void MainWindow::clearNodePaletteHelp() {
    updateHelpForSelection();
}

void MainWindow::updateThemeActions() const {
    const ThemePreset preset = m_themeManager->configuration().preset;
    if (m_darkThemeAction) {
        m_darkThemeAction->setChecked(preset == ThemePreset::Dark);
    }
    if (m_lightThemeAction) {
        m_lightThemeAction->setChecked(preset == ThemePreset::Light);
    }
    if (m_customThemeAction) {
        m_customThemeAction->setChecked(preset == ThemePreset::Custom);
    }
}
