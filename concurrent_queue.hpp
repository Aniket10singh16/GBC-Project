#ifndef FINAL_YEAR_PROJECT_CONCURRENT_QUEUE_HPP
#define FINAL_YEAR_PROJECT_CONCURRENT_QUEUE_HPP
#include <pthread.h>
#include <deque>
#include <mutex>
#include <condition_variable>
namespace tq {
    template<typename T>

    class ThreadQueue {
    public:
        ThreadQueue() {
            pthread_mutex_init(&m_qmtx, nullptr);
            pthread_cond_init(&m_condv, nullptr);
        }

        ~ThreadQueue() {
            pthread_mutex_lock(&m_qmtx);
            m_queue.clear();
            pthread_mutex_unlock(&m_qmtx);
        }

        void clear() {
            m_queue.clear();
        }

        bool empty()
        {
            return m_queue.empty();
        }

        void push_back(T t_data) {
            pthread_mutex_lock(&m_qmtx);
            m_queue.push_back(t_data);
            pthread_mutex_unlock(&m_qmtx);

            pthread_cond_signal(&m_condv);
        }

        T front() {
            T ret;
            pthread_mutex_lock(&m_qmtx);
            while (m_queue.empty()) {
                pthread_cond_wait(&m_condv, &m_qmtx);
            }
            ret = m_queue.front();
            pthread_mutex_unlock(&m_qmtx);
            return ret;
        }

        void pop_front() {
            pthread_mutex_lock(&m_qmtx);
            if (!m_queue.empty())
                m_queue.pop_front();
            pthread_mutex_unlock(&m_qmtx);
        }

    private:
        std::deque<T> m_queue;
        pthread_mutex_t m_qmtx{};
        pthread_cond_t m_condv{};
    };
}
#endif //FINAL_YEAR_PROJECT_CONCURRENT_QUEUE_HPP
