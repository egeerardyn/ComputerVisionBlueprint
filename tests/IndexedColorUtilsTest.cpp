#include <QImage>
#include <QColor>

#include "Nodes/Images/IndexedColorUtils.h"

namespace {
int run() {
    QImage rgb(32, 20, QImage::Format_RGB888);
    for (int y = 0; y < rgb.height(); ++y) {
        for (int x = 0; x < rgb.width(); ++x) {
            rgb.setPixelColor(x, y, QColor((x * 7) % 255, (y * 13) % 255, ((x + y) * 3) % 255));
        }
    }

    const QImage indexed = ConvertToIndexed8(rgb);
    if (indexed.isNull() || indexed.format() != QImage::Format_Indexed8 || indexed.width() != rgb.width() ||
        indexed.height() != rgb.height()) {
        return 1;
    }

    const QImage convertedBack = ConvertFromIndexedToRgb(indexed);
    if (convertedBack.isNull() || convertedBack.format() != QImage::Format_RGB888 ||
        convertedBack.width() != rgb.width() || convertedBack.height() != rgb.height()) {
        return 2;
    }

    return 0;
}
} // namespace

int main() {
    return run();
}
