//
// Created by nikit on 16.04.2025.
//

#ifndef DATABASE_H
#define DATABASE_H
#include "common.hpp"

class DataBaseLog {
private:
    pqxx::connection m_conn;
    std::mutex m_mtx;
    std::queue<std::function<void(pqxx::work &)> > m_queue;
    std::atomic<bool> m_running{true};
    std::thread m_worker_thread;
};

#endif //DATABASE_H
