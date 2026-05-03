#ifndef INDEXEDCOLORUTILS_H
#define INDEXEDCOLORUTILS_H

#include <QImage>

QImage ConvertToIndexed8(const QImage& source);

QImage ConvertFromIndexedToRgb(const QImage& source);

#endif // INDEXEDCOLORUTILS_H
