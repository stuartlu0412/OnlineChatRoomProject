#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <atomic>
#include "ThreadPool.h"

// -----------------------------------------------------------------------------
// Test 1: ThreadPool Executes All Tasks
// -----------------------------------------------------------------------------
TEST(ThreadPoolTest, ExecutesAllTasks) {
    const size_t NUM_THREADS = 4;
    const size_t NUM_TASKS = 10;

    ThreadPool pool(NUM_THREADS);
    std::atomic<int> taskCounter{0};

    // Enqueue tasks that increment the taskCounter
    for (size_t i = 0; i < NUM_TASKS; ++i) {
        pool.enqueue([&taskCounter]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Simulate work
            ++taskCounter;
        });
    }

    // Wait for tasks to finish
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Verify all tasks were executed
    EXPECT_EQ(taskCounter.load(), NUM_TASKS);
}

// -----------------------------------------------------------------------------
// Test 2: ThreadPool Handles Concurrent Enqueues
// -----------------------------------------------------------------------------
TEST(ThreadPoolTest, HandlesConcurrentEnqueues) {
    const size_t NUM_THREADS = 4;
    const size_t NUM_TASKS = 50;

    ThreadPool pool(NUM_THREADS);
    std::atomic<int> taskCounter{0};

    // Simulate multiple threads enqueueing tasks
    std::vector<std::thread> enqueueThreads;
    for (size_t i = 0; i < NUM_THREADS; ++i) {
        enqueueThreads.emplace_back([&pool, &taskCounter, NUM_TASKS]() {
            for (size_t j = 0; j < NUM_TASKS / NUM_THREADS; ++j) {
                pool.enqueue([&taskCounter]() {
                    std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Simulate work
                    ++taskCounter;
                });
            }
        });
    }

    // Wait for all enqueue threads to finish
    for (auto& t : enqueueThreads) {
        t.join();
    }

    // Allow the thread pool to process all tasks
    std::this_thread::sleep_for(std::chrono::seconds(3));

    // Verify all tasks were executed
    EXPECT_EQ(taskCounter.load(), NUM_TASKS);
}

// -----------------------------------------------------------------------------
// Test 3: ThreadPool Stops Gracefully
// -----------------------------------------------------------------------------
TEST(ThreadPoolTest, StopsGracefully) {
    const size_t NUM_THREADS = 4;
    const size_t NUM_TASKS = 20;

    ThreadPool pool(NUM_THREADS);
    std::atomic<int> taskCounter{0};

    // Enqueue tasks that take some time to execute
    for (size_t i = 0; i < NUM_TASKS; ++i) {
        pool.enqueue([&taskCounter]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Simulate work
            ++taskCounter;
        });
    }

    // Allow partial processing
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Destructor will stop the pool and join threads
    // When the destructor is called, tasks in progress should complete
    // but new tasks will not start
}

// -----------------------------------------------------------------------------
// Main Function for Google Test
// -----------------------------------------------------------------------------
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
