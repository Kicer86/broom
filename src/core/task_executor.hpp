/*
 * An aplication wide universal task executor. Runs on all available cpu cores
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

#ifndef TASKEXECUTOR_HPP
#define TASKEXECUTOR_HPP

#include <thread>

#include "core_export.h"
#include "itask_executor.hpp"
#include "ts_multi_head_queue.hpp"

class ILogger;

struct CORE_EXPORT TaskExecutor: public ITaskExecutor
{
    TaskExecutor(ILogger *);
    TaskExecutor(const TaskExecutor &) = delete;
    virtual ~TaskExecutor();

    TaskExecutor& operator=(const TaskExecutor &) = delete;

    void add(std::unique_ptr<ITask> &&) override;
    TaskQueue getCustomTaskQueue() override;
    void stop() override;

    void eat();

private:
    typedef TS_MultiHeadQueue<std::unique_ptr<ITask>> QueueT;
    QueueT m_tasks;
    std::unique_ptr<QueueT::Producer> m_producer;
    std::thread m_taskEater;
    ILogger* m_logger;
    bool m_working;

    void execute(const std::shared_ptr<ITask>& task) const;
};


#endif // TASKEXECUTOR_HPP
