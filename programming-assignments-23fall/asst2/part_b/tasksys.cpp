#include "tasksys.h"
#include <iostream>
#include <algorithm>
#include <cassert>


IRunnable::~IRunnable() {}

ITaskSystem::ITaskSystem(int num_threads) {}
ITaskSystem::~ITaskSystem() {}

/*
 * ================================================================
 * Serial task system implementation
 * ================================================================
 */

const char* TaskSystemSerial::name() {
    return "Serial";
}

TaskSystemSerial::TaskSystemSerial(int num_threads): ITaskSystem(num_threads) {
}

TaskSystemSerial::~TaskSystemSerial() {}

void TaskSystemSerial::run(IRunnable* runnable, int num_total_tasks) {
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemSerial::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                          const std::vector<TaskID>& deps) {
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemSerial::sync() {
    return;
}

/*
 * ================================================================
 * Parallel Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelSpawn::name() {
    return "Parallel + Always Spawn";
}

TaskSystemParallelSpawn::TaskSystemParallelSpawn(int num_threads): ITaskSystem(num_threads) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
}

TaskSystemParallelSpawn::~TaskSystemParallelSpawn() {}

void TaskSystemParallelSpawn::run(IRunnable* runnable, int num_total_tasks) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemParallelSpawn::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                 const std::vector<TaskID>& deps) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemParallelSpawn::sync() {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    return;
}

/*
 * ================================================================
 * Parallel Thread Pool Spinning Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelThreadPoolSpinning::name() {
    return "Parallel + Thread Pool + Spin";
}

TaskSystemParallelThreadPoolSpinning::TaskSystemParallelThreadPoolSpinning(int num_threads): ITaskSystem(num_threads) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
}

TaskSystemParallelThreadPoolSpinning::~TaskSystemParallelThreadPoolSpinning() {}

void TaskSystemParallelThreadPoolSpinning::run(IRunnable* runnable, int num_total_tasks) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemParallelThreadPoolSpinning::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                              const std::vector<TaskID>& deps) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemParallelThreadPoolSpinning::sync() {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
    return;
}

/*
 * ================================================================
 * Parallel Thread Pool Sleeping Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelThreadPoolSleeping::name() {
    return "Parallel + Thread Pool + Sleep";
}

void TaskSystemParallelThreadPoolSleeping::Scheduler::NotifyTaskFinished(const TaskID id)
{
    m_finishedSet.insert(id);
    // 获取所有依赖于该任务的子任务
    auto it = m_reverseDeps.find(id);
    if (it == m_reverseDeps.end())
    {
        return;
    }
    for (const auto &childID : it->second)
    {
        // 减少子任务的未完成依赖计数
        if(m_remainingDeps.find(childID) == m_remainingDeps.end()) {
            continue;
        }
        m_remainingDeps[childID]--;
        // 如果子任务的所有依赖都已完成，则将其移到就绪队列
        if (m_remainingDeps[childID] == 0)
        {
            auto taskIt = m_waitingMap.find(childID);
            if (taskIt != m_waitingMap.end())
            {
                m_readyQueue.push(taskIt->second);
                m_waitingMap.erase(taskIt);
            }
        }
    }
    m_notifyCv.notify_all();
}

void TaskSystemParallelThreadPoolSleeping::Scheduler::UpdateWaitingTasks()
{
    for (auto it = m_waitingMap.begin(); it != m_waitingMap.end();)
    {
        auto taskParents = it->second.parents;
        if (std::all_of(taskParents.begin(), taskParents.end(),
                        [this](TaskID parentID) { return m_finishedSet.count(parentID) > 0; }))
        {
            std::cout<<"Moving Task " << it->second.id << " from waiting to ready queue." << std::endl;
            m_readyQueue.push(it->second);
            it = m_waitingMap.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void TaskSystemParallelThreadPoolSleeping::Scheduler::AddTask(const TaskID id, IRunnable *runnable,
                                                              const std::vector<TaskID> &parents, const int numTasks)
{
    std::lock_guard<std::recursive_mutex> lock(m_containerMtx);
    TaskInfo taskInfo(id, runnable, parents, numTasks);
    std::cout<<"Adding Task " << id << " with " << parents.size() << " dependencies." << std::endl;
    // If no dependencies, add to ready queue
    if (parents.empty())
    {
        m_readyQueue.push(taskInfo);
        std::cout<<"Task " << id << " has no dependencies, added to ready queue." << std::endl;
        m_notifyCv.notify_all();
        return;
    }

    std::cout<<"Task " << id << " has dependencies, added to waiting map." << std::endl;
    m_waitingMap.emplace(id, taskInfo);
    m_remainingDeps[id] = parents.size();

    // 更新反向依赖关系
    for (const auto &parent : parents)
    {
        m_reverseDeps[parent].push_back(id);
    }
    UpdateWaitingTasks();
    m_notifyCv.notify_all();
}

bool TaskSystemParallelThreadPoolSleeping::Scheduler::GetTask(TaskInfo &taskInfo)
{
    std::unique_lock<std::recursive_mutex> lock(m_containerMtx);
    // 等待直到有任务可用或系统停止
    std::cout<<"Worker waiting for tasks..." << std::endl;
    m_notifyCv.wait(lock, [this]() { return !IsEmpty() || m_isStopRef; });
    std::cout<<"Worker woke up." << std::endl;
    if (m_isStopRef)
    {
        std::cout<<"Worker stopping as system is stopping." << std::endl;
        return false;
    }
    // task finished check
    while (!IsEmpty() && m_readyQueue.front().IsFinished())
    {
        TaskInfo finishedTask = m_readyQueue.front();
        NotifyTaskFinished(finishedTask.id);
        m_readyQueue.pop();
        std::cout<<"Task " << finishedTask.id << " finished." << std::endl;
    }
    // get available task
    if (!m_readyQueue.empty())
    {
        taskInfo = m_readyQueue.front();
        return true;
    }
    // refresh waiting tasks
    while (m_waitingMap.size() > 0)
    {
        UpdateWaitingTasks();
    }

    std::cout<<"Unexpected available task to fetch." << std::endl;
    return false;
}

bool TaskSystemParallelThreadPoolSleeping::Scheduler::IsEmpty()
{
    std::lock_guard<std::recursive_mutex> lock(m_containerMtx);
    return m_readyQueue.empty() && m_waitingMap.empty();
}

void TaskSystemParallelThreadPoolSleeping::Scheduler::NotifyAll()
{
    m_notifyCv.notify_all();
}

void TaskSystemParallelThreadPoolSleeping::Scheduler::NotifyOne()
{
    m_notifyCv.notify_one();
}

void TaskSystemParallelThreadPoolSleeping::WorkersSpawn(const int num_threads)
{
    for (int i = 0; i < num_threads; i++)
    {
        m_workers.emplace_back([this]() {
            while (!m_isStop)
            {
                TaskInfo taskInfo(0, nullptr, {}, 0);
                if (this->m_scheduler.GetTask(taskInfo))
                {
                    int taskIdx = taskInfo.numTasks - taskInfo.taskRemain->fetch_sub(1);
                    if(taskIdx < 0 || taskIdx >= taskInfo.numTasks) {
                        continue;
                    }

                    std::cout << "Worker executing task " << taskInfo.id << " Task :" << taskIdx << "/"
                              << taskInfo.numTasks << std::endl;
                    taskInfo.runnable->runTask(taskIdx, taskInfo.numTasks);
                }
            }
        });
    }
}

void TaskSystemParallelThreadPoolSleeping::WorkersDestroy()
{
    m_isStop = true;
    m_scheduler.NotifyAll();
    for (auto &worker : m_workers)
    {
        if (worker.joinable())
        {
            worker.join();
        }
    }
}

TaskSystemParallelThreadPoolSleeping::TaskSystemParallelThreadPoolSleeping(int num_threads)
    : ITaskSystem(num_threads), m_numWorkers(num_threads), m_isStop(false),  m_scheduler(m_isStop),m_nextTaskID(0)
{
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    WorkersSpawn(num_threads);
}

TaskSystemParallelThreadPoolSleeping::~TaskSystemParallelThreadPoolSleeping() {
    //
    // TODO: CS149 student implementations may decide to perform cleanup
    // operations (such as thread pool shutdown construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    WorkersDestroy();
}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int num_total_tasks) {
    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //
    runAsyncWithDeps(runnable, num_total_tasks, {});
    sync();
}

TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                    const std::vector<TaskID>& deps) {


    //
    // TODO: CS149 students will implement this method in Part B.
    //

    m_scheduler.AddTask(m_nextTaskID, runnable, deps, num_total_tasks);
    return m_nextTaskID++;
}

void TaskSystemParallelThreadPoolSleeping::sync() {

    //
    // TODO: CS149 students will modify the implementation of this method in Part B.
    //
    std::cout<<"Sync called, waiting for all tasks to complete..." << std::endl;
    while (!m_scheduler.IsEmpty()) {
        std::this_thread::yield();
    }
    std::cout<<"All tasks completed." << std::endl;

    return;
}
