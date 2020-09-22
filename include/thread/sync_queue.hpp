//
// Created by yinguohua on 2019/9/24.
//
#pragma once

#include "utility/comm_macro.h"
#include <deque>
#include <mutex>
#include <thread>
#include <condition_variable>

using namespace std;

template<typename T>
class SyncQueue {
public:
    SyncQueue(size_t maxSize) : m_maxSize(maxSize), m_needStop(false) {
    }

    void Put(const T &x) {
        Add(x);
    }

    void Put(T &&x) {
        Add(std::forward<T>(x));
    }

    void Take(std::deque<T> &list) {
        std::unique_lock<std::mutex> locker(m_mutex);
        m_notEmpty.wait(locker, [this]
        { return m_needStop || NotEmpty(); });

        if (m_needStop)
        {
            return;
        }
        list = std::move(m_queue);
        m_notFull.notify_one();
    }

    void Take(T &t) {
        std::unique_lock<std::mutex> locker(m_mutex);
        m_notEmpty.wait(locker, [this]
        { return m_needStop || NotEmpty(); });

        if (m_needStop)
        {
            return;
        }
        t = m_queue.front();
        m_queue.pop_front();
        m_notFull.notify_one();
    }

    void Stop() {
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            m_needStop = true;
        }
        m_notFull.notify_all();
        m_notEmpty.notify_all();
    }

    bool Empty() {
        std::lock_guard<std::mutex> locker(m_mutex);
        return m_queue.empty();
    }

    bool Full() {
        std::lock_guard<std::mutex> locker(m_mutex);
        return m_queue.size() == m_maxSize;
    }

    size_t Size() {
        std::lock_guard<std::mutex> locker(m_mutex);
        return m_queue.size();
    }

    std::int32_t Count() {
        std::lock_guard<std::mutex> locker(m_mutex);
        return m_queue.size();
    }

private:
    bool NotFull() const {
        bool full = m_queue.size() >= m_maxSize;
        if (full)
        {
            LOG_ERROR("full, waiting，thread id: {}", (syscall(__NR_gettid)));
        }
        return !full;
    }

    bool NotEmpty() const {
        bool empty = m_queue.empty();
//        if (empty)
//        {
//            LOG_ERROR("empty,waiting，thread id: {}", (syscall(__NR_gettid)));
//        }
        return !empty;
    }

    template<typename F>
    void Add(F &&x) {
        std::unique_lock<std::mutex> locker(m_mutex);
        if (m_needStop || !NotFull())
        {
            return;
        }

        m_queue.push_back(std::forward<F>(x));
        m_notEmpty.notify_one();
    }

private:
    std::deque<T> m_queue; //缓冲区
    std::mutex m_mutex; //互斥量和条件变量结合起来使用
    std::condition_variable m_notEmpty;//不为空的条件变量
    std::condition_variable m_notFull; //没有满的条件变量
    size_t m_maxSize; //同步队列最大的size
    bool m_needStop; //停止的标志
};

