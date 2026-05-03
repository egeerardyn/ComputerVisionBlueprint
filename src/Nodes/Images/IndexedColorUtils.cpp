#include "IndexedColorUtils.h"

namespace {
QVector<QRgb> GrayColorTable() {
    QVector<QRgb> table;
    table.reserve(256);
    for (int i = 0; i < 256; ++i) {
        table.push_back(qRgb(i, i, i));
    }
    return table;
}
} // namespace

QImage ConvertToIndexed8(const QImage& source) {
    if (source.isNull()) {
        return {};
    }

    QImage indexed = source.convertToFormat(QImage::Format_Indexed8, Qt::AvoidDither | Qt::NoOpaqueDetection);
    if (indexed.colorCount() == 0) {
        indexed.setColorTable(GrayColorTable());
    }

    return indexed;
}

QImage ConvertFromIndexedToRgb(const QImage& source) {
    if (source.isNull()) {
        return {};
    }

    if (source.format() != QImage::Format_Indexed8) {
        return source.convertToFormat(QImage::Format_RGB888);
    }

    return source.convertToFormat(QImage::Format_RGB888);
}
