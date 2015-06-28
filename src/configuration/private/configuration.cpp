/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Michał Walenciak <MichalWalenciak@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "configuration.hpp"
#include "private/configuration_p.hpp"

#include <QFile>

#include <system/system.hpp>


ConfigurationPrivate::ConfigurationPrivate(Configuration* q):
    m_json(),
    q(q),
    m_loaded(false)
{
}


ConfigurationPrivate::~ConfigurationPrivate()
{
}


void ConfigurationPrivate::ensureDataLoaded()
{
    if (m_loaded == false)
    {
        loadData();
        m_loaded = true;
    }
}


void ConfigurationPrivate::loadData()
{
    const QString path = System::getApplicationConfigDir();
    const QString configFile = path + "/" + "config.json";
    QFile config(configFile);

    if (QFile::exists(configFile))
    {
        config.open(QIODevice::ReadOnly);

        const QByteArray data = config.readAll();
        m_json = QJsonDocument::fromJson(data);
    }
}


////////////////////////////////////////////////////////////////////////////////


Configuration::Configuration():
    d(new ConfigurationPrivate(this))
{

}


Configuration::~Configuration()
{
    delete d;
}


ConfigurationAccessor Configuration::get()
{

}
