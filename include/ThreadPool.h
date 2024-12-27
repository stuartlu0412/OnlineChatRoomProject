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
using Socket = int;

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
     * @brief Enqueues a new socket (file descriptor) for processing by a worker thread.
     *
     * @param socketFd The socket descriptor to be added to the queue.
     */
    void enqueue(Socket socketFd);

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

    /**
     * @brief Total number of threads in the pool.
     */
    size_t numThreads_;

    /**
     * @brief Array (or vector) of pthread_t handles for the worker threads.
     */
    std::vector<pthread_t> threads_;

    /**
     * @brief Atomic flag indicating whether the pool is stopping.
     */
    std::atomic_bool stop_;

    /**
     * @brief The queue of pending socket descriptors.
     */
    std::queue<Socket> socketQueue_;

    /**
     * @brief POSIX mutex to protect access to socketQueue_.
     */
    pthread_mutex_t queueMutex_;

    /**
     * @brief Condition variable (pthread version) to notify workers
     *        when new sockets arrive in the queue.
     */
    pthread_cond_t condition_;
};

#endif // THREADPOOL_H
