
#include "qml_utils.hpp"

#include <QQuickWidget>
#include <QQuickItem>
#include <QQmlContext>
#include <QQmlPropertyMap>

namespace QmlUtils
{
    QObject* findQmlObject(QQuickWidget* qml, const QString& objectName)
    {
        auto rootObject = qml->rootObject();
        return (rootObject->objectName() == objectName)?
            rootObject :
            rootObject->findChild<QObject*>(objectName);
    }


    void registerObject(QQuickWidget* qml, const QString& objectName, QObject* object)
    {
        auto rootContext = qml->rootContext();
        rootContext->setContextProperty(objectName, object);
    }


    void registerObjectProperties(QQuickWidget* qml, const QString& objectName, QQmlPropertyMap* properties)
    {
        auto rootContext = qml->rootContext();
        rootContext->setContextProperty(objectName, properties);
    }
}
