

#ifndef ORDERMANAGER_H
#define ORDERMANAGER_H

#include "common.hpp"
#include "BinanceAPI.h"
#include "AccountManager.h"
#include "DataBase.h"

using std::queue;
using std::mutex;
using std::thread;

using std::atomic;
using namespace std::chrono;

using std::cout;
using std::cerr;
using std::endl;



class OrderManager {
private:
    struct Order {
        string action;
        string symbol;
        double price;
        double quant;
    };

    BinanceAPIc& Api;
    AccountManager& Acc;
    DataBaseLog& logger;

    queue<Order> order_q;
    mutex mutex_ord;
    thread stream_t;

    atomic<bool> run;

    int oreder_rate;

    void process_orders();

public:
    OrderManager(BinanceAPIc& api, AccountManager& acc_mgr, DataBaseLog& log, int rate_lim): Api(api), Acc(acc_mgr) ,oreder_rate(rate_lim), logger(log) {}

    void start();

    void stop();

    void add_order(const string& action, const string& symbol, double price, double quant);

    size_t queue_size();
};


#endif //ORDERMANAGER_H
