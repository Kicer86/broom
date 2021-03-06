
#include <QImage>
#include <QPainter>
#include <QSvgRenderer>

#include "svg_utils.hpp"


namespace SVGUtils
{
    QImage load(const QString& path,  const QSize& size, const QColor& background)
    {
        QSvgRenderer renderer(path);
        QImage image(size, QImage::Format_ARGB32);
        image.fill(background);

        QPainter painter(&image);
        renderer.render(&painter);

        return image;
    }
}
