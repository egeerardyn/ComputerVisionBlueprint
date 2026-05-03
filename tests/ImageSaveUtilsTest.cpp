#include <QTemporaryDir>
#include <QFileInfo>
#include <QColor>

#include "Nodes/Images/ImageSaveUtils.h"

namespace {
int run() {
    QTemporaryDir dir;
    if (!dir.isValid()) {
        return 1;
    }

    QImage image(40, 30, QImage::Format_RGB888);
    image.fill(QColor(12, 34, 56));

    const QString pngPath = dir.path() + "/test_output.png";
    if (!SaveImageToFile(image, pngPath, "PNG", -1)) {
        return 2;
    }
    if (!QFileInfo::exists(pngPath)) {
        return 3;
    }

    const QString bmpPath = dir.path() + "/test_output.bmp";
    if (!SaveImageToFile(image, bmpPath, "BMP", -1)) {
        return 4;
    }
    if (!QFileInfo::exists(bmpPath)) {
        return 5;
    }

    if (SaveImageToFile(QImage(), pngPath, "PNG", -1)) {
        return 6;
    }

    return 0;
}
} // namespace

int main() {
    return run();
}
