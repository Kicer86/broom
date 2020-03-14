
#ifndef QML_UTILS_HPP
#define QML_UTILS_HPP

#include <QString>
#include <QVariant>

class QQuickWidget;

namespace QmlUtils
{
    QObject* findQmlObject(QQuickWidget *, const QString& objectName);
}

#endif