#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <condition_variable>
#include <functional>
#include <future>
#include <limits>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <vector>

class ThreadPoolQueueFull : public std::runtime_error
{
  public:
    ThreadPoolQueueFull() : std::runtime_error("ThreadPool queue is full")
    {
    }
};

class ThreadPool
{
  public:
    explicit ThreadPool(size_t threads,
                        size_t max_waiting_tasks = std::numeric_limits<size_t>::max());
    template <class F, class... Args>
    auto enqueue(F &&f, Args &&...args) -> std::future<typename std::result_of<F(Args...)>::type>;

    template <class T> struct EnqueueResult
    {
        std::future<T> future;
        bool will_wait;
        size_t waiting_tasks;
    };

    template <class F, class... Args>
    auto enqueue_with_status(F &&f, Args &&...args)
        -> EnqueueResult<typename std::result_of<F(Args...)>::type>;

    ~ThreadPool();

  private:
    // need to keep track of threads so we can join them
    std::vector<std::thread> workers;
    // the task queue
    std::queue<std::function<void()>> tasks;

    // synchronization
    std::mutex queue_mutex;
    std::condition_variable condition;
    size_t active_tasks;
    size_t max_waiting_tasks;
    bool stop;
};

// the constructor just launches some amount of workers
inline ThreadPool::ThreadPool(size_t threads, size_t max_waiting_tasks)
    : active_tasks(0), max_waiting_tasks(max_waiting_tasks), stop(false)
{
    for (size_t i = 0; i < threads; ++i)
        workers.emplace_back([this] {
            for (;;) {
                std::function<void()> task;

                {
                    std::unique_lock<std::mutex> lock(this->queue_mutex);
                    this->condition.wait(lock,
                                         [this] { return this->stop || !this->tasks.empty(); });
                    if (this->stop && this->tasks.empty())
                        return;
                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                    ++this->active_tasks;
                }

                task();

                {
                    std::unique_lock<std::mutex> lock(this->queue_mutex);
                    --this->active_tasks;
                }
                this->condition.notify_all();
            }
        });
}

// add new work item to the pool
template <class F, class... Args>
auto ThreadPool::enqueue(F &&f, Args &&...args)
    -> std::future<typename std::result_of<F(Args...)>::type>
{
    return enqueue_with_status(std::forward<F>(f), std::forward<Args>(args)...).future;
}

template <class F, class... Args>
auto ThreadPool::enqueue_with_status(F &&f, Args &&...args)
    -> EnqueueResult<typename std::result_of<F(Args...)>::type>
{
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<return_type> res = task->get_future();
    bool will_wait = false;
    size_t waiting_tasks = 0;
    {
        std::unique_lock<std::mutex> lock(queue_mutex);

        // don't allow enqueueing after stopping the pool
        if (stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        if (tasks.size() >= max_waiting_tasks)
            throw ThreadPoolQueueFull();

        will_wait = !tasks.empty() || active_tasks >= workers.size();
        tasks.emplace([task]() { (*task)(); });
        waiting_tasks = tasks.size();
    }
    condition.notify_one();
    return {std::move(res), will_wait, waiting_tasks};
}

// the destructor joins all threads
inline ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for (std::thread &worker : workers)
        worker.join();
}

#endif
