#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <pthread.h>
#include <queue>
#include <mutex>
#include <atomic>

/**
 * @brief Define the type of socket we're handling (e.g., a file descriptor).
 *        Here we assume a simple "int" for the socket file descriptor.
 */

using Task = std::function<void()>;  // Define a Task as a callable

class ThreadPool
{
public:
    /**
     * @brief Constructs a ThreadPool with a given number of worker threads.
     *
     * @param numThreads Number of threads to spawn in the pool.
     */
    explicit ThreadPool(size_t numThreads);

    /**
     * @brief Destroys the ThreadPool.
     *  - Notifies worker threads to stop.
     *  - Joins all worker threads.
     *  - Cleans up resources.
     */
    ~ThreadPool();

     /**
     * @brief Enqueues a new task for the worker threads.
     * 
     * @param task A callable task to be executed.
     */
    void enqueue(Task task);


private:
    /**
     * @brief Static worker function that each thread will run.
     *  - Because pthread_create expects a C-style function pointer,
     *    we make this static and pass 'this' via `arg`.
     *
     * @param arg Pointer to the ThreadPool instance (cast back to `ThreadPool*`).
     */
    static void* workerFunc(void* arg);

    /**
     * @brief Main loop for a single worker thread.
     *  - Waits for a socket to become available.
     *  - Pops the socket from the queue.
     *  - Processes the connection (e.g., by creating a Connection object).
     */
    void workerLoop();

    size_t numThreads_;
    std::vector<pthread_t> threads_;
    std::atomic_bool stop_;
    std::queue<Task> taskQueue_;        // Task queue
    pthread_mutex_t queueMutex_;
    pthread_cond_t condition_;
};

#endif // THREADPOOL_H
