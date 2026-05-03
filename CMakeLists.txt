cmake_minimum_required(VERSION 3.8)
project(OpenCVNodeEditor)

set(CMAKE_CXX_STANDARD 17)

find_package(Qt6 COMPONENTS Widgets Gui Concurrent Multimedia Network REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

#find opencv
find_package(OpenCV CONFIG REQUIRED)

add_subdirectory(3rdparty/nodeeditor)

add_executable(ComputerVisionBlueprint
        resources/images/images.qrc
        src/main.cpp
        #src/Nodes/OpenCV/FacialDetectorModel.cpp
        #src/Nodes/OpenCV/FacialDetectorModel.h
        #src/Nodes/OpenCV/RectangleForm.ui
        #src/Nodes/OpenCV/RectangleModel.cpp
        #src/Nodes/OpenCV/RectangleModel.h
        src/CustomWidget/CheckBoxReadOnly.cpp
        src/CustomWidget/CheckBoxReadOnly.h
        src/Nodes/Basic/AdditionModel.h
        src/Nodes/Basic/AdditionModel.cpp
        src/Nodes/Basic/DivisionModel.h
        src/Nodes/Basic/DivisionModel.cpp
        src/Nodes/Basic/MathOperationDataModel.cpp
        src/Nodes/Basic/MathOperationDataModel.hpp
        src/Nodes/Basic/MultiplicationModel.cpp
        src/Nodes/Basic/MultiplicationModel.hpp
        src/Nodes/Basic/NumberDisplayDataModel.cpp
        src/Nodes/Basic/NumberDisplayDataModel.hpp
        src/Nodes/Basic/NumberSourceDataModel.cpp
        src/Nodes/Basic/NumberSourceDataModel.hpp
        src/Nodes/Basic/SubtractionModel.cpp
        src/Nodes/Basic/SubtractionModel.h
        src/Nodes/Constants/PiModel.cpp
        src/Nodes/Constants/PiModel.h
        src/Nodes/Conversor/MatQt.cpp
        src/Nodes/Conversor/MatQt.h
        src/Nodes/NodeHelpInfo.cpp
        src/Nodes/NodeHelpInfo.h
        src/Nodes/Data/CascadeClassifierData.h
        src/Nodes/Data/CircleData.h
        src/Nodes/Data/CirclesData.h
        src/Nodes/Data/DataInclude.h
        src/Nodes/Data/FileData.h
        src/Nodes/Data/ImageData.h
        src/Nodes/Data/ImageFormatData.h
        src/Nodes/Data/LinesSegmentData.h
        src/Nodes/Data/MarkersData.h
        src/Nodes/Data/RectsData.h
        src/Nodes/Data/VariantData.h
        src/Nodes/Images/DrawLinesForm.ui
        src/Nodes/Images/DrawLinesModel.cpp
        src/Nodes/Images/DrawLinesModel.h
        src/Nodes/Images/DrawCirclesForm.ui
        src/Nodes/Images/DrawCirclesModel.cpp
        src/Nodes/Images/DrawCirclesModel.h
        src/Nodes/Images/DrawRectsForm.ui
        src/Nodes/Images/DrawRectsModel.cpp
        src/Nodes/Images/DrawRectsModel.h
        src/Nodes/Images/ImageInfoForm.ui
        src/Nodes/Images/ImageInfoModel.cpp
        src/Nodes/Images/ImageInfoModel.h
        src/Nodes/Images/ImageLoaderModel.cpp
        src/Nodes/Images/ImageLoaderModel.hpp
        src/Nodes/Images/ImageShowModel.cpp
        src/Nodes/Images/ImageShowModel.hpp
        src/Nodes/NodesInclude.cpp
        src/Nodes/NodesInclude.h
        src/Nodes/OpenCV/BlurForm.ui
        src/Nodes/OpenCV/BlurModel.cpp
        src/Nodes/OpenCV/BlurModel.h
        src/Nodes/OpenCV/CannyForm.ui
        src/Nodes/OpenCV/CannyModel.cpp
        src/Nodes/OpenCV/CannyModel.h
        src/Nodes/OpenCV/CascadeClassifierForm.ui
        src/Nodes/OpenCV/CascadeClassifierModel.cpp
        src/Nodes/OpenCV/CascadeClassifierModel.h
        src/Nodes/OpenCV/BilateralFilterForm.ui
        src/Nodes/OpenCV/BilateralFilterModel.cpp
        src/Nodes/OpenCV/BilateralFilterModel.h
        src/Nodes/OpenCV/BoxFilterForm.ui
        src/Nodes/OpenCV/BoxFilterModel.cpp
        src/Nodes/OpenCV/BoxFilterModel.h
        src/Nodes/OpenCV/ColorCVModel.cpp
        src/Nodes/OpenCV/ColorCVModel.h
        src/Nodes/OpenCV/DetectMultiScaleModel.cpp
        src/Nodes/OpenCV/DetectMultiScaleModel.h
        src/Nodes/OpenCV/EqualizeHistModel.cpp
        src/Nodes/OpenCV/EqualizeHistModel.h
        src/Nodes/OpenCV/Filter2DModel.cpp
        src/Nodes/OpenCV/Filter2DModel.h
        src/Nodes/OpenCV/DistanceTransformModel.cpp
        src/Nodes/OpenCV/DistanceTransformModel.h
        src/Nodes/OpenCV/ConvertDepthChannelsModel.cpp
        src/Nodes/OpenCV/ConvertDepthChannelsModel.h
        src/Nodes/OpenCV/TypeConversionUtils.cpp
        src/Nodes/OpenCV/TypeConversionUtils.h
        src/Nodes/OpenCV/MorphologyModelBase.cpp
        src/Nodes/OpenCV/MorphologyModelBase.h
        src/Nodes/OpenCV/ErodeModel.cpp
        src/Nodes/OpenCV/ErodeModel.h
        src/Nodes/OpenCV/DilateModel.cpp
        src/Nodes/OpenCV/DilateModel.h
        src/Nodes/OpenCV/MorphologyExModel.cpp
        src/Nodes/OpenCV/MorphologyExModel.h
        src/Nodes/OpenCV/GaussianBlurForm.ui
        src/Nodes/OpenCV/GaussianBlurModel.cpp
        src/Nodes/OpenCV/GaussianBlurModel.h
        src/Nodes/OpenCV/HoughLinesPModel.cpp
        src/Nodes/OpenCV/HoughLinesPModel.h
        src/Nodes/OpenCV/HoughLinesPForm.ui
        src/Nodes/OpenCV/HoughCirclesModel.cpp
        src/Nodes/OpenCV/HoughCirclesModel.h
        src/Nodes/OpenCV/HoughCirclesUtils.cpp
        src/Nodes/OpenCV/HoughCirclesUtils.h
        src/Nodes/OpenCV/GradientUtils.cpp
        src/Nodes/OpenCV/GradientUtils.h
        src/Nodes/OpenCV/SobelGradientModel.cpp
        src/Nodes/OpenCV/SobelGradientModel.h
        src/Nodes/OpenCV/ScharrGradientModel.cpp
        src/Nodes/OpenCV/ScharrGradientModel.h
        src/Nodes/OpenCV/HistogramClaheUtils.cpp
        src/Nodes/OpenCV/HistogramClaheUtils.h
        src/Nodes/OpenCV/HistogramModel.cpp
        src/Nodes/OpenCV/HistogramModel.h
        src/Nodes/OpenCV/ClaheModel.cpp
        src/Nodes/OpenCV/ClaheModel.h
        src/Nodes/OpenCV/TemplateMatchingUtils.cpp
        src/Nodes/OpenCV/TemplateMatchingUtils.h
        src/Nodes/OpenCV/TemplateMatchingModel.cpp
        src/Nodes/OpenCV/TemplateMatchingModel.h
        src/Nodes/OpenCV/MedianBlurForm.ui
        src/Nodes/OpenCV/MedianBlurModel.cpp
        src/Nodes/OpenCV/MedianBlurModel.h
        src/Nodes/OpenCV/OpenCVFilterUtils.h
        src/Nodes/OpenCV/WatershedModel.cpp
        src/Nodes/OpenCV/WatershedModel.h
        src/Nodes/Variables/FileVarForm.ui
        src/Nodes/Variables/FileVarModel.cpp
        src/Nodes/Variables/FileVarModel.h
        src/Nodes/Variables/SizeVarForm.ui
        src/Nodes/Variables/SizeVarModel.cpp
        src/Nodes/Variables/SizeVarModel.h
        src/Widgets/MainWindow.cpp
        src/Widgets/MainWindow.h
        src/Widgets/MainWindow.ui
        src/Widgets/HelpWidget.cpp
        src/Widgets/HelpWidget.h
        src/Widgets/ThemeControlsWidget.cpp
        src/Widgets/ThemeControlsWidget.h
        src/Widgets/ThemeManager.cpp
        src/Widgets/ThemeManager.h
        src/Nodes/Variables/RectVarModel.cpp
        src/Nodes/Variables/RectVarModel.h
        src/Nodes/Variables/CircleForm.ui
        src/Nodes/Variables/CircleModel.cpp
        src/Nodes/Variables/CircleModel.h
        src/Nodes/Variables/CircleVarModel.cpp
        src/Nodes/Variables/CircleVarModel.h
        src/Nodes/Data/PointData.h
        src/Nodes/Data/RectData.h
        src/Nodes/Images/ConvertImageToModel.cpp
        src/Nodes/Images/ConvertImageToModel.h
        src/Nodes/Images/ConvertImageToForm.ui
        src/Nodes/Images/ScaleImageModel.cpp
        src/Nodes/Images/ScaleImageModel.h
        src/Nodes/Images/ScaleImageForm.ui
        src/Nodes/DataOperations/ScaleRects.cpp
        src/Nodes/DataOperations/ScaleRects.h
        src/Nodes/Variables/RectModel.cpp
        src/Nodes/Variables/RectModel.h
        src/Nodes/Variables/RectForm.ui
        src/Nodes/Images/CutImageModel.cpp
        src/Nodes/Images/CutImageModel.h
        src/Nodes/Video/CameraModel.cpp
        src/Nodes/Video/CameraModel.h
        src/Nodes/Video/CameraForm.ui
        src/Nodes/Video/CaptureModel.cpp
        src/Nodes/Video/CaptureModel.h
        src/CustomWidget/DragableTreeOfNodes.cpp
        src/CustomWidget/DragableTreeOfNodes.h
        src/CustomWidget/DropGraphicsView.cpp
        src/CustomWidget/DropGraphicsView.h
        src/Nodes/OpenCV/PyrDown.cpp
        src/Nodes/OpenCV/PyrDown.h
        src/Nodes/OpenCV/PyrUp.cpp
        src/Nodes/OpenCV/PyrUp.h
        src/Nodes/OpenCV/SqrBoxFilterModel.cpp
        src/Nodes/OpenCV/SqrBoxFilterModel.h
        src/Nodes/Variables/FileFromUrlModel.cpp
        src/Nodes/Variables/FileFromUrlModel.h
        src/Nodes/Variables/FileFromUrlForm.ui
)

target_link_libraries(ComputerVisionBlueprint PRIVATE
        QtNodes::QtNodes
        ${OpenCV_LIBS}
        Qt6::Concurrent
        Qt6::Multimedia
        Qt6::Network
)

target_include_directories(ComputerVisionBlueprint PRIVATE src)
target_include_directories(ComputerVisionBlueprint PRIVATE 3rdparty/nodeeditor/src)
target_include_directories(ComputerVisionBlueprint PRIVATE 3rdparty/nodeeditor/include/QtNodes/internal)

enable_testing()

add_executable(ComputerVisionBlueprintTests
        tests/ConversionSmokeTest.cpp
        src/Nodes/Conversor/MatQt.cpp
)

target_link_libraries(ComputerVisionBlueprintTests PRIVATE
        ${OpenCV_LIBS}
        Qt6::Gui
)

target_include_directories(ComputerVisionBlueprintTests PRIVATE src)

add_test(NAME conversion_smoke_test COMMAND ComputerVisionBlueprintTests)

add_executable(ComputerVisionBlueprintTypeConversionTests
        tests/TypeConversionUtilsTest.cpp
        src/Nodes/OpenCV/TypeConversionUtils.cpp
)

target_link_libraries(ComputerVisionBlueprintTypeConversionTests PRIVATE
        ${OpenCV_LIBS}
)

target_include_directories(ComputerVisionBlueprintTypeConversionTests PRIVATE src)

add_test(NAME type_conversion_utils_test COMMAND ComputerVisionBlueprintTypeConversionTests)

add_executable(ComputerVisionBlueprintHoughCirclesTests
        tests/HoughCirclesUtilsTest.cpp
        src/Nodes/OpenCV/HoughCirclesUtils.cpp
)

target_link_libraries(ComputerVisionBlueprintHoughCirclesTests PRIVATE
        ${OpenCV_LIBS}
)

target_include_directories(ComputerVisionBlueprintHoughCirclesTests PRIVATE src)

add_test(NAME hough_circles_utils_test COMMAND ComputerVisionBlueprintHoughCirclesTests)

add_executable(ComputerVisionBlueprintGradientTests
        tests/GradientUtilsTest.cpp
        src/Nodes/OpenCV/GradientUtils.cpp
)

target_link_libraries(ComputerVisionBlueprintGradientTests PRIVATE
        ${OpenCV_LIBS}
)

target_include_directories(ComputerVisionBlueprintGradientTests PRIVATE src)

add_test(NAME gradient_utils_test COMMAND ComputerVisionBlueprintGradientTests)

add_executable(ComputerVisionBlueprintHistogramClaheTests
        tests/HistogramClaheUtilsTest.cpp
        src/Nodes/OpenCV/HistogramClaheUtils.cpp
)

target_link_libraries(ComputerVisionBlueprintHistogramClaheTests PRIVATE
        ${OpenCV_LIBS}
)

target_include_directories(ComputerVisionBlueprintHistogramClaheTests PRIVATE src)

add_test(NAME histogram_clahe_utils_test COMMAND ComputerVisionBlueprintHistogramClaheTests)

add_executable(ComputerVisionBlueprintTemplateMatchingTests
        tests/TemplateMatchingUtilsTest.cpp
        src/Nodes/OpenCV/TemplateMatchingUtils.cpp
)

target_link_libraries(ComputerVisionBlueprintTemplateMatchingTests PRIVATE
        ${OpenCV_LIBS}
)

target_include_directories(ComputerVisionBlueprintTemplateMatchingTests PRIVATE src)

add_test(NAME template_matching_utils_test COMMAND ComputerVisionBlueprintTemplateMatchingTests)
