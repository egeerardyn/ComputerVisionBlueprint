#include "ImageSaveUtils.h"

#include <QDir>
#include <QFileInfo>

bool SaveImageToFile(const QImage& image,
                     const QString& path,
                     const QByteArray& format,
                     const int quality) {
    if (image.isNull() || path.isEmpty()) {
        return false;
    }

    const QFileInfo info(path);
    if (!info.dir().exists()) {
        QDir dir;
        if (!dir.mkpath(info.dir().absolutePath())) {
            return false;
        }
    }

    return image.save(path, format.constData(), quality);
}
