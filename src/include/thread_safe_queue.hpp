// File: include/thread_safe_queue.hpp
// Author(s): Caleb Johnson-Cantrell, Carlos López Ramírez, Ojas Nadkarni

#pragma once

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <queue>

// Represents a thread-safe queue meant for use with this application
template<typename T>
class ThreadSafeQueue {
  public:
    // Constructs a new thread-safe queue
    ThreadSafeQueue() = default;

    // Make this queue non-copyable
    ThreadSafeQueue(const ThreadSafeQueue &other) = delete;

    // Destructs this thread-safe queue
    ~ThreadSafeQueue() { wake_all_threads(); }

    // Returns a read-only reference to the element at the front of the queue
    T &front() {
        std::scoped_lock q_lock(sync_mux_);
        return queue_.front();
    }

    // Returns a read-only reference to the element at the back of the queue
    T &back() {
        std::scoped_lock q_lock(sync_mux_);
        return queue_.back();
    }

    // Adds the specified `element` to the back of the queue
    void push(T &element) {
        std::scoped_lock q_lock(sync_mux_);
        queue_.push(std::move(element));

        std::unique_lock cv_lock(wait_mux_);
        cv_.notify_all();
    }

    // Removes and returns the element from the front of the queue
    T pop() {
        std::scoped_lock q_lock(sync_mux_);
        T element = std::move(queue_.front());
        queue_.pop();
        cv_.notify_one();
        return element;
    }

    // Returns true if the queue contains no elements
    bool empty() {
        std::scoped_lock q_lock(sync_mux_);
        return queue_.empty();
    }

    // Returns the size of the queue
    size_t size() {
        std::scoped_lock q_lock(sync_mux_);
        return queue_.size();
    }

    // Clears the queue of all internal elements
    void clear() {
        std::scoped_lock q_lock(sync_mux_);
        std::queue<T> empty_queue;
        std::swap(queue_, empty_queue);
    }

    // Waits until an element is added to the queue
    void wait() {
        while (empty()) {
            std::unique_lock cv_lock(wait_mux_);
            cv_.wait(cv_lock);
        }
    }

    // Waits for an element to be added to the queue with a timeout of `timeout_millis` milliseconds
    void wait_for(unsigned int timeout_millis) {
        std::cv_status status = std::cv_status::no_timeout;
        while ((status != std::cv_status::timeout) && empty()) {
            std::unique_lock cv_lock(wait_mux_);
            status = cv_.wait_for(cv_lock, std::chrono::milliseconds(timeout_millis));
        }
    }

    // Force all threads that are waiting on this queue to wake up.
    void wake_all_threads() {
        std::unique_lock cv_lock(wait_mux_);
        cv_.notify_all();
    }

  private:
    // The internal container manipulated by this queue
    std::queue<T> queue_;

    // The mutex used for thread synchronization in this queue
    std::mutex sync_mux_;

    // The mutex used to block execution during the waiting period of the `wait()` function
    std::mutex wait_mux_;

    // The condition variable used for waiting on this queue
    std::condition_variable cv_;
};