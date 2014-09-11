
#include "mainwindow.hpp"

#include <QCloseEvent>
#include <QMenuBar>
#include <QFileDialog>

#include <database/database_builder.hpp>
#include <database/idatabase.hpp>
#include <project_utils/iproject_manager.hpp>
#include <project_utils/iproject.hpp>

#include "centralwidget.hpp"
#include "components/project_creator/project_creator_dialog.hpp"


MainWindow::MainWindow(QWidget *p): QMainWindow(p), m_prjManager(nullptr), m_pluginLoader(nullptr), m_currentPrj(nullptr), m_centralWidget(nullptr)
{
    m_centralWidget = new CentralWidget(this);
    setCentralWidget(m_centralWidget);

    QMenu* projectMenu = new QMenu( tr("Project"), this);
    QAction* newPrjAction  = projectMenu->addAction( tr("New project") );
    QAction* openPrjAction = projectMenu->addAction( tr("Open project") );

    QMenuBar* mainMenuBar = new QMenuBar(this);
    mainMenuBar->addMenu(projectMenu);

    setMenuBar(mainMenuBar);

    connect( newPrjAction,  SIGNAL(triggered(bool)), this, SLOT(newProject()) );
    connect( openPrjAction, SIGNAL(triggered(bool)), this, SLOT(openProject()) );
}


MainWindow::~MainWindow()
{
}


void MainWindow::set(IProjectManager* prjManager)
{
    m_prjManager = prjManager;
}


void MainWindow::set(IPluginLoader* pluginLoader)
{
    m_pluginLoader = pluginLoader;
}


void MainWindow::closeEvent(QCloseEvent *e)
{
    // TODO: close project!
    //m_currentPrj->close();

    e->accept();
}


void MainWindow::openProject(const QString& prjFile)
{
    std::shared_ptr<IProject> prj = m_prjManager->open(prjFile);
    m_currentPrj = prj;
    Database::IDatabase* db = m_currentPrj->getDatabase();

    m_centralWidget->setDatabase(db);
}


void MainWindow::newProject()
{
    ProjectCreatorDialog prjCreatorDialog;
    prjCreatorDialog.set(m_pluginLoader);
    const int status = prjCreatorDialog.exec();

    if (status == QDialog::Accepted)
    {
        const QString prjPath   = prjCreatorDialog.getPrjPath();
        const auto*   prjPlugin = prjCreatorDialog.getEnginePlugin();

        const bool creation_status = m_prjManager->new_prj(prjPath, prjPlugin);

        if (creation_status)
            openProject(prjPath);
    }
}


void MainWindow::openProject()
{
    const QString prjFile = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                         "",
                                                         tr("Broom projects (*.bpj)"));

    openProject(prjFile);
}

