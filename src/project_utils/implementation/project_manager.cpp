/*
 * Photo Broom - photos management tool.
 * Copyright (C) 2014  Michał Walenciak <MichalWalenciak@gmail.com>
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

#include "project_manager.hpp"

#include <cassert>

#include <QSettings>
#include <QFileInfo>
#include <QDir>

#include <core/base_tags.hpp>
#include <database/idatabase_builder.hpp>
#include <database/project_info.hpp>
#include <database/idatabase.hpp>
#include <plugins/iplugin_loader.hpp>
#include <system/system.hpp>

#include "project.hpp"


ProjectManager::ProjectManager(Database::IBuilder* builder): m_dbBuilder(builder)
{

}


ProjectInfo ProjectManager::new_prj(const QString& prjName, const Database::IPlugin* prjPlugin, const QString& location)
{
    QDir().mkpath(location);

    const QDir storagePath(location);
    const QString prjPath = storagePath.absoluteFilePath(prjName + ".bpj");
    const ProjectInfo prjInfo(prjPath);
    const QString dbDir = prjInfo.getInternalLocation(ProjectInfo::Database);

    QDir().mkpath(dbDir);

    //prepare database
    const Database::ProjectInfo dbPrjInfo = prjPlugin->initPrjDir(dbDir, prjName);

    // construct relative path to database location
    const QDir baseDir(prjInfo.getBaseDir());
    const QString databaseLocation = baseDir.relativeFilePath(dbPrjInfo.databaseLocation);

    //prepare project file
    QSettings prjFile(prjPath, QSettings::IniFormat);

    prjFile.beginGroup("Database");
    prjFile.setValue("backend", dbPrjInfo.backendName);
    prjFile.setValue("location", databaseLocation);
    prjFile.endGroup();

    prjFile.beginGroup("Project");
    prjFile.setValue("name", prjName);
    prjFile.endGroup();

    return prjInfo;
}


ProjectManager::OpenStatus ProjectManager::open(const ProjectInfo& prjInfo)
{
    std::unique_ptr<Project> project = std::make_unique<Project>(nullptr, prjInfo);
    Database::BackendStatus db_status;

    const QString& prjPath = prjInfo.getPath();
    const bool prjFileExists = QFile::exists(prjPath);

    if (prjFileExists)
    {
        const QString basePath = prjInfo.getBaseDir();

        QSettings prjFile(prjPath, QSettings::IniFormat);

        prjFile.beginGroup("Database");
        const QString backend  = prjFile.value("backend").toString();
        const QString location = basePath + "/" + prjFile.value("location").toString();
        prjFile.endGroup();

        Database::ProjectInfo dbPrjInfo(location, backend);
        auto db = m_dbBuilder->get(dbPrjInfo);

        project = std::make_unique<Project>(std::move(db), prjInfo);

        const bool lock_status = project->lockProject();

        if (lock_status)
        {
            QEventLoop loop;

            project->getDatabase().init(dbPrjInfo, [&loop, &db_status](const Database::BackendStatus& status)
            {
                // this lambda function can be called from an unknown thread (even main thread).
                // it is not guaranteed that `loop` already works.
                // Make sure we will quit it by creating a delayed slot call.
                // TODO: tricky...
                QObject obj;
                QObject::connect(&obj, &QObject::destroyed, &loop, &QEventLoop::quit, Qt::QueuedConnection);

                db_status = status;
            });

            loop.exec();
        }
        else
        {
            db_status = Database::StatusCodes::ProjectLocked;
            project = std::make_unique<Project>(nullptr, prjInfo);
        }
    }
    else
        db_status = Database::StatusCodes::OpenFailed;

    return std::make_pair(std::move(project), db_status);
}
