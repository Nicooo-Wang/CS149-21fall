#include "tasksys.h"
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

IRunnable::~IRunnable()
{
}

ITaskSystem::ITaskSystem(int num_threads)
{
}
ITaskSystem::~ITaskSystem()
{
}

/*
 * ================================================================
 * Serial task system implementation
 * ================================================================
 */

const char *TaskSystemSerial::name()
{
    return "Serial";
}

TaskSystemSerial::TaskSystemSerial(int num_threads) : ITaskSystem(num_threads)
{
}

TaskSystemSerial::~TaskSystemSerial()
{
}

void TaskSystemSerial::run(IRunnable *runnable, int num_total_tasks)
{
    for (int i = 0; i < num_total_tasks; i++)
    {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemSerial::runAsyncWithDeps(IRunnable *runnable, int num_total_tasks, const std::vector<TaskID> &deps)
{
    // You do not need to implement this method.
    return 0;
}

void TaskSystemSerial::sync()
{
    // You do not need to implement this method.
    return;
}

/*
 * ================================================================
 * Parallel Task System Implementation
 * ================================================================
 */

const char *TaskSystemParallelSpawn::name()
{
    return "Parallel + Always Spawn";
}

TaskSystemParallelSpawn::TaskSystemParallelSpawn(int num_threads) : ITaskSystem(num_threads), m_num_threads(num_threads)
{
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
}

TaskSystemParallelSpawn::~TaskSystemParallelSpawn()
{
}

void TaskSystemParallelSpawn::run(IRunnable *runnable, int num_total_tasks)
{

    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //

    std::vector<std::thread> threads;

    int num_tasks_per_thread = (num_total_tasks + m_num_threads - 1) / m_num_threads;

    for (int i = 0; i < num_total_tasks; i += num_tasks_per_thread)
    {
        threads.emplace_back([runnable, i, num_tasks_per_thread, num_total_tasks]() {
            for (int j = i; j < std::min(num_total_tasks, i + num_tasks_per_thread); j++)
            {
                runnable->runTask(j, num_total_tasks);
            }
        });
    }

    for (auto &thread : threads)
    {
        thread.join();
    }
}

TaskID TaskSystemParallelSpawn::runAsyncWithDeps(IRunnable *runnable, int num_total_tasks,
                                                 const std::vector<TaskID> &deps)
{
    // You do not need to implement this method.
    return 0;
}

void TaskSystemParallelSpawn::sync()
{
    // You do not need to implement this method.
    return;
}

/*
 * ================================================================
 * Parallel Thread Pool Spinning Task System Implementation
 * ================================================================
 */

const char *TaskSystemParallelThreadPoolSpinning::name()
{
    return "Parallel + Thread Pool + Spin";
}

void TaskSystemParallelThreadPoolSpinning::WorkersSpawn(const int num_threads)
{
    for (int i = 0; i < num_threads; i++)
    {
        m_workers.emplace_back([this] {
            while (true)
            {
                std::unique_lock<std::mutex> lock(m_mtx);
                this->m_cv.wait(lock, [this] { return m_isStop || m_taskRemain; });
                if (m_isStop && !m_taskRemain)
                {
                    return;
                }
                m_taskRemain--;
                int curTask = m_taskRemain;
                lock.unlock();
                m_runnable->runTask(curTask, m_runnableNum);

                int finished = ++m_taskFinished;
                if (finished == m_runnableNum) {
                    m_completion_cv.notify_one();
                }
            }
        });
    }
}

void TaskSystemParallelThreadPoolSpinning::WorkersDestory()
{
    m_isStop = true;
    m_cv.notify_all();
    for (auto &worker : m_workers)
    {
        if (worker.joinable())
        {
            worker.join();
        }
    }
}

TaskSystemParallelThreadPoolSpinning::TaskSystemParallelThreadPoolSpinning(int num_threads)
    : ITaskSystem(num_threads), m_numWorkers(num_threads), m_taskRemain(0), m_isStop(false)
{
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    WorkersSpawn(num_threads);
}

TaskSystemParallelThreadPoolSpinning::~TaskSystemParallelThreadPoolSpinning()
{
    WorkersDestory();
}

void TaskSystemParallelThreadPoolSpinning::run(IRunnable *runnable, int num_total_tasks)
{

    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //
    m_taskRemain = num_total_tasks;
    m_taskFinished = 0;
    m_runnableNum = num_total_tasks;
    m_runnable = runnable;

    m_cv.notify_all();

    // 使用条件变量等待任务完成，避免忙等待
    std::unique_lock<std::mutex> lock(m_completionMtx);
    m_completion_cv.wait(lock, [this] { return m_taskFinished == m_runnableNum; });
}

TaskID TaskSystemParallelThreadPoolSpinning::runAsyncWithDeps(IRunnable *runnable, int num_total_tasks,
                                                              const std::vector<TaskID> &deps)
{
    // You do not need to implement this method.
    return 0;
}

void TaskSystemParallelThreadPoolSpinning::sync()
{
    // You do not need to implement this method.
    return;
}

/*
 * ================================================================
 * Parallel Thread Pool Sleeping Task System Implementation
 * ================================================================
 */

const char *TaskSystemParallelThreadPoolSleeping::name()
{
    return "Parallel + Thread Pool + Sleep";
}


TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(IRunnable *runnable, int num_total_tasks,
                                                              const std::vector<TaskID> &deps)
{

    //
    // TODO: CS149 students will implement this method in Part B.
    //

    return 0;
}

void TaskSystemParallelThreadPoolSleeping::sync()
{

    //
    // TODO: CS149 students will modify the implementation of this method in Part B.
    //

    return;
}
