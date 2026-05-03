#ifndef IMAGESAVEUTILS_H
#define IMAGESAVEUTILS_H

#include <QImage>
#include <QString>

bool SaveImageToFile(const QImage& image,
                     const QString& path,
                     const QByteArray& format,
                     int quality);

#endif // IMAGESAVEUTILS_H
