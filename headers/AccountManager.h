
#ifndef ACCOUNTMANAGER_H
#define ACCOUNTMANAGER_H
#include "common.hpp"
#include "BinanceAPI.h"

using std::unordered_map;

using std::endl;


using std::mutex;
using std::thread;
using std::lock_guard;

using std::atomic;

class AccountManager {
private:
    BinanceAPIc& Api;

    unordered_map<string, double> balance;
    mutable mutex mutex_;
    thread stram_t;

    atomic<bool> run;
    double profit;

    void runing();

    void update_balance() ;

    void process_order(const json& order);

public:

    AccountManager(BinanceAPIc& api)
       : Api(api), run(false), profit(0.0) {}

    void start();

    void stop();

    double get_balance(const std::string& asset);

    double get_profit();

    void on_order_executed(const json& order_response);
};
#endif //ACCOUNTMANAGER_H
