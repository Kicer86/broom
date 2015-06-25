/*
 * Widget for list of tasks.
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

#include "tasks_view_widget.hpp"

#include <cassert>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QProgressBar>
#include <QLabel>

#include <core/iview_task.hpp>


struct ProgressBar final: IProgressBar
{
    ProgressBar(QProgressBar* pb): m_progressBar(pb) {}
    ProgressBar(const ProgressBar &) = delete;

    ProgressBar& operator=(const ProgressBar &) = delete;

    void setMaximum(int v) override
    {
        m_progressBar->setMaximum(v);
    }

    void setMinimum(int v) override
    {
        m_progressBar->setMinimum(v);
    }

    void setValue(int v) override
    {
        m_progressBar->setValue(v);
    }

    QProgressBar* m_progressBar;
};


struct TasksViewWidget::Task: QWidget, IViewTask
{
    Task(const QString& name, TasksViewWidget* parent):
        QWidget(parent),
        IViewTask(),
        m_name(name),
        m_progressBar(new QProgressBar(this)),
        m_parent(parent),
        m_progressBarInterface(m_progressBar)
    {
        QLabel* label = new QLabel(name, this);
        label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

        QHBoxLayout* l = new QHBoxLayout(this);
        l->addWidget(m_progressBar, 30);
        l->addWidget(label, 70);
    }

    Task(const Task &) = delete;
    Task& operator=(const Task &) = delete;

    const QString& getName() override
    {
        return m_name;
    }

    IProgressBar* getProgressBar() override
    {
        return &m_progressBarInterface;
    }

    void finished() override
    {
        m_parent->finished(this);
    }

    QString m_name;
    QProgressBar* m_progressBar;
    TasksViewWidget* m_parent;
    ProgressBar m_progressBarInterface;
};


TasksViewWidget::TasksViewWidget(QWidget* p): QWidget(p), m_view(nullptr), m_tasks(0)
{
    m_view = new QScrollArea(this);
    m_view->setWidgetResizable(true);

    QWidget* mainWidget = new QWidget(this);
    QVBoxLayout* wl = new QVBoxLayout(mainWidget);
    wl->addStretch();

    m_view->setWidget(mainWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_view);
}


TasksViewWidget::~TasksViewWidget()
{

}


IViewTask* TasksViewWidget::add(const QString& name)
{
    Task* task = new Task(name, this);

    getLayout()->insertWidget(m_tasks, task);
    m_tasks++;

    return task;
}


void TasksViewWidget::finished(IViewTask* task)
{
    Task* t = static_cast<Task *>(task);

    delete t;
    m_tasks--;
}


QBoxLayout* TasksViewWidget::getLayout()
{
    QWidget* w = m_view->widget();
    QLayout* l = w->layout();

    assert(dynamic_cast<QBoxLayout*>(l));
    QBoxLayout *bl = static_cast<QBoxLayout *>(l);

    return bl;
}
