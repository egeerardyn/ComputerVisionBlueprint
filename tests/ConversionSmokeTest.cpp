#include <QImage>
#include <QColor>

#include "Nodes/Conversor/MatQt.h"

namespace {
int run() {
    // RGB888 conversion should produce a valid 3-channel matrix.
    QImage rgb(16, 12, QImage::Format_RGB888);
    rgb.fill(QColor(10, 20, 30));
    const cv::Mat rgbMat = QImageToMat(rgb);
    if (rgbMat.empty() || rgbMat.type() != CV_8UC3 || rgbMat.cols != rgb.width() || rgbMat.rows != rgb.height()) {
        return 1;
    }

    // Indexed image conversion should produce a valid grayscale matrix.
    QImage indexed(8, 8, QImage::Format_Indexed8);
    QVector<QRgb> colorTable;
    colorTable.reserve(256);
    for (int i = 0; i < 256; ++i) {
        colorTable.push_back(qRgb(i, i, i));
    }
    indexed.setColorTable(colorTable);
    indexed.fill(42);
    const cv::Mat indexedMat = QImageToMat(indexed);
    if (indexedMat.empty() || indexedMat.type() != CV_8UC1 || indexedMat.cols != indexed.width() ||
        indexedMat.rows != indexed.height()) {
        return 2;
    }

    // Round-trip for grayscale should stay valid.
    QImage gray(10, 10, QImage::Format_Grayscale8);
    gray.fill(128);
    const cv::Mat grayMat = QImageToMat(gray);
    const QImage roundTrip = MatToQImage(grayMat);
    if (roundTrip.isNull() || roundTrip.width() != gray.width() || roundTrip.height() != gray.height()) {
        return 3;
    }

    // ARGB images should convert to a valid 4-channel matrix.
    QImage argb(11, 9, QImage::Format_ARGB32);
    argb.fill(QColor(4, 8, 16, 200));
    const cv::Mat argbMat = QImageToMat(argb);
    if (argbMat.empty() || argbMat.type() != CV_8UC4 || argbMat.cols != argb.width() || argbMat.rows != argb.height()) {
        return 4;
    }

    // Less common formats should still convert through the fallback path.
    QImage mono(7, 5, QImage::Format_Mono);
    mono.fill(1);
    const cv::Mat monoMat = QImageToMat(mono);
    if (monoMat.empty() || monoMat.type() != CV_8UC4 || monoMat.cols != mono.width() || monoMat.rows != mono.height()) {
        return 5;
    }

    return 0;
}
} // namespace

int main() {
    return run();
}
