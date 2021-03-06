
#ifndef BRIDGE_HPP
#define BRIDGE_HPP

#include <QObject>

#include <database/idatabase.hpp>

class Bridge: public QObject
{
    Q_OBJECT

    Q_PROPERTY(Database::IDatabase* database READ database WRITE setDatabase NOTIFY databaseChanged)

    public:
        Bridge(QObject* parent = nullptr);
        ~Bridge() = default;

        void setDatabase(Database::IDatabase *);
        Database::IDatabase* database() const;

    signals:
        void databaseChanged(Database::IDatabase *) const;

    private:
        Database::IDatabase* m_database;
};

#endif
