#ifndef WORK_THREAD_HPP
#define WORK_THREAD_HPP

#include <thread>
#include <future>
#include <functional>
#include <atomic>
#include <mutex>

class WorkThread {
private:
    std::thread worker;
    std::mutex mtx;
    std::atomic<bool> running;
    std::atomic<bool> hasResult;
    std::future<void> taskFuture;
    std::exception_ptr taskException;

public:
    WorkThread() : running(false), hasResult(false) {}

    ~WorkThread() {
        if (worker.joinable()) worker.join();
    }

    // Launch a background task (callable with no args)
    bool start(std::function<void()> task) {
        std::lock_guard<std::mutex> lock(mtx);
        if (running) return false; // already running

        running = true;
        hasResult = false;
        taskException = nullptr;

        worker = std::thread([this, task]() {
            try {
                task();
            } catch (...) {
                taskException = std::current_exception();
            }

            running = false;
            hasResult = true;
        });

        worker.detach(); // auto-clean thread when done
        return true;
    }

    // Returns true while the task is still running
    bool isRunning() const {
        return running.load();
    }

    // Returns true if the task has completed (successfully or with exception)
    bool isFinished() const {
        return hasResult.load();
    }

    // Optional: rethrow any exception from the background thread
    void checkError() {
        if (taskException) std::rethrow_exception(taskException);
    }

    // Reset flags (call when you’ve handled the result)
    void reset() {
        hasResult = false;
        taskException = nullptr;
    }
};

#endif
