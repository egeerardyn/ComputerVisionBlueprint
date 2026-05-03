#include "NodeHelpInfo.h"

#include "Nodes/NodesInclude.h"

namespace {
    const QString kProjectUrl = QStringLiteral("https://github.com/PabloPicose/ComputerVisionBlueprint");
    const QString kOpenCvCoreImgProcUrl = QStringLiteral("https://docs.opencv.org/4.x/d7/dbd/group__imgproc.html");
    const QString kOpenCvFilterUrl = QStringLiteral("https://docs.opencv.org/4.x/d4/d86/group__imgproc__filter.html");
    const QString kOpenCvTransformUrl = QStringLiteral("https://docs.opencv.org/4.x/da/d54/group__imgproc__transform.html");
    const QString kOpenCvColorUrl = QStringLiteral("https://docs.opencv.org/4.x/d8/d01/group__imgproc__color__conversions.html");
    const QString kOpenCvDrawUrl = QStringLiteral("https://docs.opencv.org/4.x/d6/d6e/group__imgproc__draw.html");
    const QString kOpenCvCannyUrl = QStringLiteral("https://docs.opencv.org/4.x/da/d22/tutorial_py_canny.html");
    const QString kOpenCvHistogramUrl = QStringLiteral("https://docs.opencv.org/4.x/d5/daf/tutorial_py_histogram_equalization.html");
    const QString kOpenCvHoughLinesUrl = QStringLiteral("https://docs.opencv.org/4.x/d9/db0/tutorial_hough_lines.html");
    const QString kOpenCvMiscUrl = QStringLiteral("https://docs.opencv.org/4.x/d7/d1b/group__imgproc__misc.html");
    const QString kOpenCvCascadeUrl = QStringLiteral("https://docs.opencv.org/4.x/d1/de5/classcv_1_1CascadeClassifier.html");
    const QString kOpenCvWatershedUrl = QStringLiteral("https://docs.opencv.org/4.x/d3/db4/tutorial_py_watershed.html");
    const QString kOpenCvPyramidsUrl = QStringLiteral("https://docs.opencv.org/4.x/d4/d1f/tutorial_pyramids.html");
    const QString kQtCameraUrl = QStringLiteral("https://doc.qt.io/qt-6/qcamera.html");
    const QString kQtNetworkUrl = QStringLiteral("https://doc.qt.io/qt-6/qnetworkaccessmanager.html");

    NodeHelpInfo makeHelp(const QString& summary, const QString& url = QString()) {
        return {summary, QUrl(url)};
    }
}

NodeHelpInfo nodeHelpInfoForModelName(const QString& modelName) {
    if (modelName == PiModel().name()) {
        return makeHelp("Outputs the constant pi as a numeric value so other nodes can reuse it in formulas and size calculations.",
                        kProjectUrl);
    }
    if (modelName == NumberSourceDataModel().name()) {
        return makeHelp("Creates a numeric value inside the graph. Use it to feed arithmetic nodes or any parameter input that expects a scalar.",
                        kProjectUrl);
    }
    if (modelName == NumberDisplayDataModel().name()) {
        return makeHelp("Displays the numeric result arriving at its input. This is useful for inspecting intermediate values while building a workflow.",
                        kProjectUrl);
    }
    if (modelName == AdditionModel().name()) {
        return makeHelp("Adds two compatible values such as numbers or sizes and outputs the combined result.", kProjectUrl);
    }
    if (modelName == SubtractionModel().name()) {
        return makeHelp("Subtracts the second input from the first input and outputs the difference.", kProjectUrl);
    }
    if (modelName == MultiplicationModel().name()) {
        return makeHelp("Multiplies two compatible values and outputs the product. Use it for scaling factors and simple calculations.",
                        kProjectUrl);
    }
    if (modelName == DivisionModel().name()) {
        return makeHelp("Divides the first input by the second input and outputs the quotient.", kProjectUrl);
    }

    if (modelName == ImageLoaderModel().name()) {
        return makeHelp("Loads an image from disk into the graph so downstream nodes can inspect, transform, or display it.", kProjectUrl);
    }
    if (modelName == ImageShowModel().name()) {
        return makeHelp("Displays an incoming image inside the node so you can preview the current result at any stage of the pipeline.",
                        kProjectUrl);
    }
    if (modelName == ImageInfoModel().name()) {
        return makeHelp("Extracts basic metadata about the input image, such as dimensions and format, for inspection or downstream logic.",
                        kProjectUrl);
    }
    if (modelName == DrawCirclesModel().name()) {
        return makeHelp("Draws detected or user-provided circles on top of an image and outputs the annotated image.", kOpenCvDrawUrl);
    }
    if (modelName == DrawLinesModel().name()) {
        return makeHelp("Draws line segments on the input image and outputs the rendered result.", kOpenCvDrawUrl);
    }
    if (modelName == DrawRectsModel().name()) {
        return makeHelp("Draws rectangles on the input image, which is useful for bounding boxes and region overlays.", kOpenCvDrawUrl);
    }
    if (modelName == ConvertImageToModel().name()) {
        return makeHelp("Converts an image into another representation or format expected by the rest of the workflow.", kOpenCvTransformUrl);
    }
    if (modelName == ScaleImageModel().name()) {
        return makeHelp("Resizes the input image using a scale factor so later processing can run on a larger or smaller version.",
                        kOpenCvTransformUrl);
    }
    if (modelName == CutImageModel().name()) {
        return makeHelp("Crops the input image to a selected rectangular region and outputs only that sub-image.", kOpenCvTransformUrl);
    }

    if (modelName == ColorCVModel().name()) {
        return makeHelp("Converts the input image between OpenCV color spaces such as BGR, grayscale, or other supported formats.",
                        kOpenCvColorUrl);
    }
    if (modelName == BlurModel().name()) {
        return makeHelp("Applies a normalized box blur to smooth noise and soften detail in the input image.", kOpenCvFilterUrl);
    }
    if (modelName == CannyModel().name()) {
        return makeHelp("Runs the Canny edge detector and outputs an edge map based on the current threshold settings.", kOpenCvCannyUrl);
    }
    if (modelName == GaussianBlurModel().name()) {
        return makeHelp("Applies Gaussian smoothing, which is often used before edge detection or segmentation.", kOpenCvFilterUrl);
    }
    if (modelName == MedianBlurModel().name()) {
        return makeHelp("Uses a median filter to reduce salt-and-pepper noise while preserving strong edges better than a simple average blur.",
                        kOpenCvFilterUrl);
    }
    if (modelName == BilateralFilterModel().name()) {
        return makeHelp("Smooths the image while preserving edges by combining spatial distance and color similarity.", kOpenCvFilterUrl);
    }
    if (modelName == BoxFilterModel().name()) {
        return makeHelp("Applies a configurable box filter for averaging neighboring pixels over a rectangular kernel.", kOpenCvFilterUrl);
    }
    if (modelName == SqrBoxFilterModel().name()) {
        return makeHelp("Computes a squared box filter, which is useful for local statistics and variance-style preprocessing.",
                        kOpenCvFilterUrl);
    }
    if (modelName == Filter2DModel().name()) {
        return makeHelp("Applies a custom 2D convolution kernel to the input image for sharpening, embossing, or other custom effects.",
                        kOpenCvFilterUrl);
    }
    if (modelName == ErodeModel().name()) {
        return makeHelp("Shrinks bright regions according to the selected structuring element. This is useful for removing small foreground noise.",
                        kOpenCvFilterUrl);
    }
    if (modelName == DilateModel().name()) {
        return makeHelp("Expands bright regions according to the selected structuring element. This is useful for filling gaps and strengthening blobs.",
                        kOpenCvFilterUrl);
    }
    if (modelName == MorphologyExModel().name()) {
        return makeHelp("Runs higher-level morphology operations such as opening, closing, gradient, top hat, or black hat.",
                        kOpenCvFilterUrl);
    }
    if (modelName == DistanceTransformModel().name()) {
        return makeHelp("Computes the distance from each foreground pixel to the nearest background pixel, which is often used before watershed segmentation.",
                        kOpenCvMiscUrl);
    }
    if (modelName == WatershedModel().name()) {
        return makeHelp("Segments an image using marker-based watershed. Provide a source image and prepared markers to split touching regions.",
                        kOpenCvWatershedUrl);
    }
    if (modelName == HoughLinesPModel().name()) {
        return makeHelp("Detects line segments with the probabilistic Hough transform and outputs the resulting line data.", kOpenCvHoughLinesUrl);
    }
    if (modelName == EqualizeHistModel().name()) {
        return makeHelp("Equalizes the histogram to improve contrast, especially on grayscale images with low dynamic range.",
                        kOpenCvHistogramUrl);
    }
    if (modelName == PyrDown().name()) {
        return makeHelp("Reduces the image resolution by one pyramid level, applying smoothing as part of the downsampling step.",
                        kOpenCvPyramidsUrl);
    }

    if (modelName == CascadeClassifier().name()) {
        return makeHelp("Loads an OpenCV cascade classifier from a file so it can be reused by detection nodes such as Detect Multi Scale.",
                        kOpenCvCascadeUrl);
    }
    if (modelName == DetectMultiScaleModel().name()) {
        return makeHelp("Runs a cascade classifier over the image and outputs the detected rectangles for faces or other trained objects.",
                        kOpenCvCascadeUrl);
    }

    if (modelName == SizeVarModel().name()) {
        return makeHelp("Creates or stores a QSize value that can be reused by nodes needing width and height style parameters.", kProjectUrl);
    }
    if (modelName == FileVarModel().name()) {
        return makeHelp("Loads a file path into the graph so nodes can consume external resources such as classifiers or other assets.",
                        kProjectUrl);
    }
    if (modelName == CircleModel().name()) {
        return makeHelp("Creates a single circle value, typically defined by center and radius, for drawing or geometric processing nodes.",
                        kProjectUrl);
    }
    if (modelName == CircleVarModel().name()) {
        return makeHelp("Collects one or more circle inputs into a circles output that can be reused by downstream drawing or analysis nodes.",
                        kProjectUrl);
    }
    if (modelName == RectVarModel().name()) {
        return makeHelp("Collects rectangle inputs into a reusable rectangles output, which is helpful when combining detections from multiple sources.",
                        kProjectUrl);
    }
    if (modelName == RectModel().name()) {
        return makeHelp("Creates a single rectangle value that can define a crop region, drawing overlay, or region of interest.", kProjectUrl);
    }
    if (modelName == FileFromUrlModel().name()) {
        return makeHelp("Downloads a file from a URL and exposes the saved result as file data for the rest of the graph.", kQtNetworkUrl);
    }

    if (modelName == CameraModel().name()) {
        return makeHelp("Captures frames from an available camera device and outputs the live image stream into the graph.", kQtCameraUrl);
    }
    if (modelName == CaptureModel().name()) {
        return makeHelp("Takes a snapshot from the incoming video stream and publishes the captured frame as an image.", kQtCameraUrl);
    }

    if (modelName == ScaleRects().name()) {
        return makeHelp("Scales incoming rectangles by the configured factor so detections can be mapped between resized and original images.",
                        kProjectUrl);
    }

    return makeHelp("No node-specific help is available for this node yet.", kProjectUrl);
}

NodeHelpInfo nodeHelpInfoForModel(const QtNodes::NodeDelegateModel& model) {
    if (const auto* provider = dynamic_cast<const NodeHelpProvider*>(&model)) {
        return provider->nodeHelpInfo();
    }

    return nodeHelpInfoForModelName(model.name());
}
