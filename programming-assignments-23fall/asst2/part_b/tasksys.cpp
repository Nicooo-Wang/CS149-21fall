#include "tasksys.h"


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
    // 获取所有依赖于该任务的子任务
    auto it = m_reverseDeps.find(id);
    if (it == m_reverseDeps.end())
    {
        return;
    }
    for (const auto &childID : it->second)
    {
        // 减少子任务的未完成依赖计数
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
    m_reverseDeps.erase(it);
}

void TaskSystemParallelThreadPoolSleeping::Scheduler::AddTask(const TaskID id, const IRunnable *runnable,
                                                              const std::vector<TaskID> &parents, const int numTasks)
{
    std::lock_guard<std::mutex> lock(m_mtx);
    TaskInfo taskInfo(id, runnable, parents, numTasks);
    // If no dependencies, add to ready queue
    if (parents.empty())
    {
        m_readyQueue.push(taskInfo);
        return;
    }

    m_waitingMap.emplace(id, taskInfo);
    m_remainingDeps[id] = parents.size();

    // 更新反向依赖关系
    for (const auto &parent : parents)
    {
        m_reverseDeps[parent].push_back(id);
    }
}

bool TaskSystemParallelThreadPoolSleeping::Scheduler::GetTask(TaskInfo &taskInfo)
{
    std::lock_guard<std::mutex> lock(m_mtx);
    if (IsEmpty())
    {
        return false;
    }
    while (!IsEmpty() && m_readyQueue.front().IsFinished())
    {
        TaskInfo finishedTask = m_readyQueue.front();
        // 完成任务时递减计数
        (*finishedTask.taskRemain)--;
        NotifyTaskFinished(finishedTask.id);
        m_readyQueue.pop();
    }
    if (!m_readyQueue.empty())
    {
        taskInfo = m_readyQueue.front();
        m_readyQueue.pop();
        return true;
    }
    return false;
}

bool TaskSystemParallelThreadPoolSleeping::Scheduler::IsEmpty()
{
    std::lock_guard<std::mutex> lock(m_mtx);
    return m_readyQueue.empty() && m_waitingMap.empty();
}

void TaskSystemParallelThreadPoolSleeping::WorkersSpawn(const int num_threads)
{
    for (int i = 0; i < num_threads; i++)
    {
    }
}

void TaskSystemParallelThreadPoolSleeping::WorkersDestroy()
{
    m_isStop = true;
    m_stopCv.notify_all();
    for (auto &worker : m_workers)
    {
        if (worker.joinable())
        {
            worker.join();
        }
    }
}

TaskSystemParallelThreadPoolSleeping::TaskSystemParallelThreadPoolSleeping(int num_threads)
    : ITaskSystem(num_threads), m_numWorkers(num_threads), m_isStop(false)
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

}

TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                    const std::vector<TaskID>& deps) {


    //
    // TODO: CS149 students will implement this method in Part B.
    //

    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemParallelThreadPoolSleeping::sync() {

    //
    // TODO: CS149 students will modify the implementation of this method in Part B.
    //

    return;
}
