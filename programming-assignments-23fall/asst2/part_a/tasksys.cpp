// ================================================================================
// Running task system grading harness... (11 total tests)
//   - Detected CPU with 16 execution contexts
//   - Task system configured to use at most 16 threads
// ================================================================================
// ================================================================================
// Executing test: super_super_light...
// Reference binary: ./runtasks_ref_linux_arm
// Results for: super_super_light
//                                         STUDENT   REFERENCE   PERF?
// [Serial]                                10.265    10.261      1.00  (OK)
// [Parallel + Always Spawn]               149.616   148.456     1.01  (OK)
// [Parallel + Thread Pool + Spin]         14.354    27.181      0.53  (OK)
// [Parallel + Thread Pool + Sleep]        14.29     13.756      1.04  (OK)
// ================================================================================
// Executing test: super_light...
// Reference binary: ./runtasks_ref_linux_arm
// Results for: super_light
//                                         STUDENT   REFERENCE   PERF?
// [Serial]                                87.953    87.995      1.00  (OK)
// [Parallel + Always Spawn]               155.464   151.118     1.03  (OK)
// [Parallel + Thread Pool + Spin]         17.3      26.811      0.65  (OK)
// [Parallel + Thread Pool + Sleep]        17.388    17.21       1.01  (OK)
// ================================================================================
// Executing test: ping_pong_equal...
// Reference binary: ./runtasks_ref_linux_arm
// Results for: ping_pong_equal
//                                         STUDENT   REFERENCE   PERF?
// [Serial]                                1408.307  1410.042    1.00  (OK)
// [Parallel + Always Spawn]               216.826   197.801     1.10  (OK)
// [Parallel + Thread Pool + Spin]         111.068   128.531     0.86  (OK)
// [Parallel + Thread Pool + Sleep]        111.303   110.083     1.01  (OK)
// ================================================================================
// Executing test: ping_pong_unequal...
// Reference binary: ./runtasks_ref_linux_arm
// Results for: ping_pong_unequal
//                                         STUDENT   REFERENCE   PERF?
// [Serial]                                1617.34   1624.296    1.00  (OK)
// [Parallel + Always Spawn]               238.782   212.57      1.12  (OK)
// [Parallel + Thread Pool + Spin]         137.563   130.14      1.06  (OK)
// [Parallel + Thread Pool + Sleep]        137.507   118.853     1.16  (OK)
// ================================================================================
// Executing test: recursive_fibonacci...
// Reference binary: ./runtasks_ref_linux_arm
// Results for: recursive_fibonacci
//                                         STUDENT   REFERENCE   PERF?
// [Serial]                                1069.621  1072.026    1.00  (OK)
// [Parallel + Always Spawn]               76.962    76.353      1.01  (OK)
// [Parallel + Thread Pool + Spin]         71.985    76.038      0.95  (OK)
// [Parallel + Thread Pool + Sleep]        71.702    70.942      1.01  (OK)
// ================================================================================
// Executing test: math_operations_in_tight_for_loop...
// Reference binary: ./runtasks_ref_linux_arm
// Results for: math_operations_in_tight_for_loop
//                                         STUDENT   REFERENCE   PERF?
// [Serial]                                540.348   540.481     1.00  (OK)
// [Parallel + Always Spawn]               790.02    764.593     1.03  (OK)
// [Parallel + Thread Pool + Spin]         94.067    120.735     0.78  (OK)
// [Parallel + Thread Pool + Sleep]        93.98     94.557      0.99  (OK)
// ================================================================================
// Executing test: math_operations_in_tight_for_loop_fewer_tasks...
// Reference binary: ./runtasks_ref_linux_arm
// Results for: math_operations_in_tight_for_loop_fewer_tasks
//                                         STUDENT   REFERENCE   PERF?
// [Serial]                                540.066   540.323     1.00  (OK)
// [Parallel + Always Spawn]               431.723   752.529     0.57  (OK)
// [Parallel + Thread Pool + Spin]         103.718   119.487     0.87  (OK)
// [Parallel + Thread Pool + Sleep]        104.069   112.205     0.93  (OK)
// ================================================================================
// Executing test: math_operations_in_tight_for_loop_fan_in...
// Reference binary: ./runtasks_ref_linux_arm
// Results for: math_operations_in_tight_for_loop_fan_in
//                                         STUDENT   REFERENCE   PERF?
// [Serial]                                277.204   277.441     1.00  (OK)
// [Parallel + Always Spawn]               104.514   99.923      1.05  (OK)
// [Parallel + Thread Pool + Spin]         27.002    32.911      0.82  (OK)
// [Parallel + Thread Pool + Sleep]        26.868    26.957      1.00  (OK)
// ================================================================================
// Executing test: math_operations_in_tight_for_loop_reduction_tree...
// Reference binary: ./runtasks_ref_linux_arm
// Results for: math_operations_in_tight_for_loop_reduction_tree
//                                         STUDENT   REFERENCE   PERF?
// [Serial]                                276.63    276.599     1.00  (OK)
// [Parallel + Always Spawn]               30.473    38.274      0.80  (OK)
// [Parallel + Thread Pool + Spin]         21.965    23.57       0.93  (OK)
// [Parallel + Thread Pool + Sleep]        21.721    21.785      1.00  (OK)
// ================================================================================
// Executing test: spin_between_run_calls...
// Reference binary: ./runtasks_ref_linux_arm
// Results for: spin_between_run_calls
//                                         STUDENT   REFERENCE   PERF?
// [Serial]                                382.148   385.809     0.99  (OK)
// [Parallel + Always Spawn]               191.305   193.831     0.99  (OK)
// [Parallel + Thread Pool + Spin]         191.208   193.141     0.99  (OK)
// [Parallel + Thread Pool + Sleep]        191.203   192.932     0.99  (OK)
// ================================================================================
// Executing test: mandelbrot_chunked...
// Reference binary: ./runtasks_ref_linux_arm
// Results for: mandelbrot_chunked
//                                         STUDENT   REFERENCE   PERF?
// [Serial]                                410.737   410.983     1.00  (OK)
// [Parallel + Always Spawn]               26.386    26.173      1.01  (OK)
// [Parallel + Thread Pool + Spin]         25.806    28.976      0.89  (OK)
// [Parallel + Thread Pool + Sleep]        25.821    25.873      1.00  (OK)
// ================================================================================
// Overall performance results
// [Serial]                                : All passed Perf
// [Parallel + Always Spawn]               : All passed Perf
// [Parallel + Thread Pool + Spin]         : All passed Perf
// [Parallel + Thread Pool + Sleep]        : All passed Perf

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
            int curTask = -1;
            while (true)
            {
                {
                    std::unique_lock<std::mutex> lock(m_mtx);
                    this->m_cv.wait(lock, [this] { return m_isStop || m_taskRemain; });
                    if (m_isStop && !m_taskRemain)
                    {
                        return;
                    }
                    if (!m_taskRemain)
                    {
                        continue;
                    }
                    m_taskRemain--;
                    curTask = m_taskRemain;
                }
                m_runnable->runTask(curTask, m_runnableNum);

                int finished = ++m_taskFinished;
                if (finished == m_runnableNum)
                {
                    std::unique_lock<std::mutex> lock(m_completionMtx);
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

    {
        std::lock_guard<std::mutex> lock(m_mtx);
        m_taskRemain = num_total_tasks;
        m_taskFinished = 0;
        m_runnableNum = num_total_tasks;
        m_runnable = runnable;
    }

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
