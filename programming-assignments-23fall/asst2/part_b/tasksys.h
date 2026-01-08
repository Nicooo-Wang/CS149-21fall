#pragma once

#include "itasksys.h"
#include <atomic>
#include <condition_variable>
#include <functional>
#include <map>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

/*
 * TaskSystemSerial: This class is the student's implementation of a
 * serial task execution engine.  See definition of ITaskSystem in
 * itasksys.h for documentation of the ITaskSystem interface.
 */
class TaskSystemSerial : public ITaskSystem
{
  public:
    TaskSystemSerial(int num_threads);
    ~TaskSystemSerial();
    const char *name();
    void run(IRunnable *runnable, int num_total_tasks);
    TaskID runAsyncWithDeps(IRunnable *runnable, int num_total_tasks, const std::vector<TaskID> &deps);
    void sync();
};

/*
 * TaskSystemParallelSpawn: This class is the student's implementation of a
 * parallel task execution engine that spawns threads in every run()
 * call.  See definition of ITaskSystem in itasksys.h for documentation
 * of the ITaskSystem interface.
 */
class TaskSystemParallelSpawn : public ITaskSystem
{
  public:
    TaskSystemParallelSpawn(int num_threads);
    ~TaskSystemParallelSpawn();
    const char *name();
    void run(IRunnable *runnable, int num_total_tasks);
    TaskID runAsyncWithDeps(IRunnable *runnable, int num_total_tasks, const std::vector<TaskID> &deps);
    void sync();
};

/*
 * TaskSystemParallelThreadPoolSpinning: This class is the student's
 * implementation of a parallel task execution engine that uses a
 * thread pool. See definition of ITaskSystem in itasksys.h for
 * documentation of the ITaskSystem interface.
 */
class TaskSystemParallelThreadPoolSpinning : public ITaskSystem
{
  public:
    TaskSystemParallelThreadPoolSpinning(int num_threads);
    ~TaskSystemParallelThreadPoolSpinning();
    const char *name();
    void run(IRunnable *runnable, int num_total_tasks);
    TaskID runAsyncWithDeps(IRunnable *runnable, int num_total_tasks, const std::vector<TaskID> &deps);
    void sync();
};

/*
 * TaskSystemParallelThreadPoolSleeping: This class is the student's
 * optimized implementation of a parallel task execution engine that uses
 * a thread pool. See definition of ITaskSystem in
 * itasksys.h for documentation of the ITaskSystem interface.
 */
class TaskSystemParallelThreadPoolSleeping : public ITaskSystem
{
  protected:
    std::vector<std::thread> m_workers;
    int m_numWorkers;
    std::atomic<bool> m_isStop;
    std::condition_variable m_stopCv;
    std::mutex m_stopMtx;
    struct TaskInfo
    {
        TaskID id;
        const IRunnable *runnable;
        std::vector<TaskID> parents; // 依赖的父节点
        int numTasks;
        std::shared_ptr<std::atomic<int>> taskRemain;   // 未完成的任务数量
        std::shared_ptr<std::atomic<int>> parentRemain; // 未完成的父节点数量

        bool IsFinished()
        {
            return *taskRemain == 0;
        }
        TaskInfo(const TaskID id_, const IRunnable *runnable_, const std::vector<TaskID> &parents_, const int numTasks_)
            : id(id_), runnable(runnable_), parents(parents_), numTasks(numTasks_),
              taskRemain(std::make_shared<std::atomic<int>>(numTasks_)),
              parentRemain(std::make_shared<std::atomic<int>>(parents_.size()))
        {
        }
    };

    // 设计完成scheduler
    class Scheduler
    {
      protected:
        std::mutex m_mtx; // 唯一锁，保护所有容器

        std::queue<TaskInfo> m_readyQueue;       // 就绪队列
        std::map<TaskID, TaskInfo> m_waitingMap; // 等待队列

        // 依赖跟踪
        std::map<TaskID, int> m_remainingDeps;               // 每个任务还需多少依赖未完成
        std::map<TaskID, std::vector<TaskID>> m_reverseDeps; // 父 -> 子列表
        void NotifyTaskFinished(const TaskID id);

      public:
        void AddTask(const TaskID id, const IRunnable *runnable, const std::vector<TaskID> &parents,
                     const int numTasks);
        bool GetTask(TaskInfo &taskInfo);
        bool IsEmpty();
    };

    Scheduler m_scheduler;

    void WorkersSpawn(int num_threads);
    void WorkersDestroy();

  public:
    TaskSystemParallelThreadPoolSleeping(int num_threads);
    ~TaskSystemParallelThreadPoolSleeping();
    const char *name();
    void run(IRunnable *runnable, int num_total_tasks);
    TaskID runAsyncWithDeps(IRunnable *runnable, int num_total_tasks, const std::vector<TaskID> &deps);
    void sync();
};