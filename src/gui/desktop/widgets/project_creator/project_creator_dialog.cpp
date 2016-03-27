/*
 * Dialog for new projects creation
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

#include "project_creator_dialog.hpp"

#include <set>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QFileDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QComboBox>
#include <QGroupBox>
#include <QStackedLayout>
#include <QString>


#include <database/idatabase_plugin.hpp>
#include <plugins/iplugin_loader.hpp>
#include <project_utils/iproject_manager.hpp>


namespace
{
    struct PluginOrder
    {
        bool operator() (const Database::IPlugin* p1, const Database::IPlugin* p2) const
        {
            return p1->simplicity() > p2->simplicity();
        }
    };
}


ProjectCreatorDialog::ProjectCreatorDialog(): QDialog(),
                                              m_chooseDialog(nullptr),
                                              m_prjName(nullptr),
                                              m_engines(nullptr),
                                              m_engineOptions(nullptr),
                                              m_pluginLoader(nullptr)
{
    setWindowTitle(tr("Photo collection creator"));
    resize(500, 250);

    //project location line
    QLabel* prjNameLabel = new QLabel(tr("Collection name:"), this);
    m_prjName = new QLineEdit(this);

    //project location line layout
    QHBoxLayout* prjNameLayout = new QHBoxLayout;
    prjNameLayout->addWidget(prjNameLabel);
    prjNameLayout->addWidget(m_prjName);

    //storage engine
    QLabel* dbEngine = new QLabel(tr("Database engine:"), this);
    m_engines = new QComboBox(this);

    //storage engine layout
    QHBoxLayout* dbEngineLayout = new QHBoxLayout;
    dbEngineLayout->addWidget(dbEngine);
    dbEngineLayout->addWidget(m_engines);

    //storage options
    m_engineOptions = new QGroupBox(tr("Engine options"));

    //default buttons
    QDialogButtonBox* defaultButtons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(defaultButtons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(defaultButtons, SIGNAL(rejected()), this, SLOT(reject()));

    //main layout
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(prjNameLayout);
    mainLayout->addLayout(dbEngineLayout);
    mainLayout->addWidget(m_engineOptions);
    mainLayout->addStretch();
    mainLayout->addWidget(defaultButtons);
}


ProjectCreatorDialog::~ProjectCreatorDialog()
{

}


void ProjectCreatorDialog::set(IPluginLoader* pluginLoader)
{
    m_pluginLoader = pluginLoader;
    initEngines();
}


QString ProjectCreatorDialog::getPrjName() const
{
    const QString prjName = m_prjName->text();

    return prjName;
}


Database::IPlugin* ProjectCreatorDialog::getEnginePlugin() const
{
    Database::IPlugin* plugin = getSelectedPlugin();

    return plugin;
}


void ProjectCreatorDialog::initEngines()
{
    const std::deque<Database::IPlugin *>& plugins = m_pluginLoader->getDBPlugins();

    std::set<Database::IPlugin *, PluginOrder> plugins_ordered;
    plugins_ordered.insert(plugins.cbegin(), plugins.cend());

    QStackedLayout* engineOptionsLayout = new QStackedLayout(m_engineOptions);

    for(auto plugin: plugins_ordered)
    {
        const QVariant itemData = QVariant::fromValue<Database::IPlugin *>(plugin);
        m_engines->addItem(plugin->backendName(), itemData);

        QWidget* optionsWidget = new QWidget(this);
        QLayout* optionsWidgetLayout = plugin->buildDBOptions();

        if (optionsWidgetLayout != nullptr)
            optionsWidget->setLayout(optionsWidgetLayout);

        engineOptionsLayout->addWidget(optionsWidget);
    }

    connect(m_engines, SIGNAL(activated(int)), engineOptionsLayout, SLOT(setCurrentIndex(int)));
}


Database::IPlugin* ProjectCreatorDialog::getSelectedPlugin() const
{
    const QVariant pluginRaw = m_engines->currentData();
    Database::IPlugin* plugin = pluginRaw.value<Database::IPlugin *>();

    return plugin;
}


///////////////////////////////////////////////////////////////////////////////


ProjectCreator::ProjectCreator(): m_prj()
{

}


bool ProjectCreator::create(IProjectManager* prjManager, IPluginLoader* pluginLoader)
{
    ProjectCreatorDialog prjCreatorDialog;
    prjCreatorDialog.set(pluginLoader);
    const int status = prjCreatorDialog.exec();

    if (status == QDialog::Accepted)
    {
        const QString prjName   = prjCreatorDialog.getPrjName();
        const auto*   prjPlugin = prjCreatorDialog.getEnginePlugin();

        m_prj = prjManager->new_prj(prjName, prjPlugin);
    }

    const bool result = m_prj.isValid();

    return result;
}


ProjectInfo ProjectCreator::project() const
{
    return m_prj;
}
