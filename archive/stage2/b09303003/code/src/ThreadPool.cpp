#include "ThreadPool.h"
#include "Connection.h"   // If you want to instantiate and handle the connection here
#include <iostream>       // For std::cerr, etc.
#include <cstring>        // For strerror
#include <cerrno>         // For errno


ThreadPool::ThreadPool(size_t numThreads)
    : numThreads_(numThreads),
      threads_(),
      stop_(false)
{
    int ret = pthread_mutex_init(&queueMutex_, nullptr);
    if (ret != 0) {
        std::cerr << "pthread_mutex_init failed: " << strerror(ret) << std::endl;
        // Handle error (throw, exit, etc.)
    }
    
    // Initialize the pthread condition variable
    ret = pthread_cond_init(&condition_, nullptr);
    if (ret != 0) {
        std::cerr << "pthread_cond_init failed: " << strerror(ret) << std::endl;
        // Handle error (throw exception, exit, etc.)
    }

    // Resize the vector to hold numThreads_ pthread_t handles
    threads_.resize(numThreads_);

    // Create each worker thread
    for (size_t i = 0; i < numThreads_; ++i) {
        ret = pthread_create(&threads_[i], nullptr, &ThreadPool::workerFunc, this);
        if (ret != 0) {
            std::cerr << "pthread_create failed: " << strerror(ret) << std::endl;
            // Handle error (throw exception, log, etc.)
        }
    }
}

ThreadPool::~ThreadPool()
{
    // Signal to stop all threads
    stop_ = true;

    // Wake up all worker threads so they can exit
    int ret = pthread_cond_broadcast(&condition_);
    if (ret != 0) {
        std::cerr << "pthread_cond_broadcast failed: " << strerror(ret) << std::endl;
        // Handle error
    }

    // Join all worker threads
    for (size_t i = 0; i < numThreads_; ++i) {
        pthread_join(threads_[i], nullptr);
    }

    // Clean up
    pthread_cond_destroy(&condition_);
    pthread_mutex_destroy(&queueMutex_);
}

void ThreadPool::enqueue(Task task)
{
    // Lock the queue mutex before modifying the queue
    int ret = pthread_mutex_lock(&queueMutex_);
    if (ret != 0) {
        std::cerr << "pthread_mutex_lock failed in enqueue: " << strerror(ret) << std::endl;
        // Handle error
    }

    taskQueue_.push(task);

    // Unlock and signal one worker thread
    ret = pthread_mutex_unlock(&queueMutex_);
    if (ret != 0) {
        std::cerr << "pthread_mutex_unlock failed in enqueue: " << strerror(ret) << std::endl;
        // Handle error
    }

    // Signal one worker that a new socket is available
    ret = pthread_cond_signal(&condition_);
    if (ret != 0) {
        std::cerr << "pthread_cond_signal failed: " << strerror(ret) << std::endl;
        // Handle error
    }
}

void* ThreadPool::workerFunc(void* arg)
{
    // Cast the arg back to our ThreadPool*
    ThreadPool* pool = static_cast<ThreadPool*>(arg);
    if (pool == nullptr) {
        return nullptr;
    }

    pool->workerLoop();
    return nullptr;
}

void ThreadPool::workerLoop()
{
    while (true) {
        Task task;

        // Acquire the lock to safely access the queue
        int ret = pthread_mutex_lock(&queueMutex_);
        if (ret != 0) {
            std::cerr << "pthread_mutex_lock failed in workerLoop: " << strerror(ret) << std::endl;
            // Possibly handle error more gracefully
        }

        // Wait until we have something in the queue OR we are stopping
        while (!stop_ && taskQueue_.empty()) {
            ret = pthread_cond_wait(&condition_, &queueMutex_);
            if (ret != 0) {
                std::cerr << "pthread_cond_wait failed: " << strerror(ret) << std::endl;
                // Typically you'd handle/log the error, but let's continue
            }
        }

        // If stop_ was set, break out of the loop
        if (stop_) {
            pthread_mutex_unlock(&queueMutex_);
            break;
        }

        // Now we should have at least one socket in the queue
        //sock = socketQueue_.front();
        //socketQueue_.pop();
        // Get the next task from the queue
        task = std::move(taskQueue_.front());
        taskQueue_.pop();

        // Unlock the mutex so other threads can continue
        ret = pthread_mutex_unlock(&queueMutex_);
        if (ret != 0) {
            std::cerr << "pthread_mutex_unlock failed in workerLoop: " << strerror(ret) << std::endl;
            // Handle error
        }

        // -------------------------------------------------------------
        // Process the socket: create a Connection and call handleClient()
        // -------------------------------------------------------------
        
        //if (sock >= 0) {
        //    Connection connection(sock, {} /* optional sockaddr_in if you have it */);
        //    connection.handleClient();
            // The connection destructor will close the socket automatically
        //}
        
        task();
    }
}
