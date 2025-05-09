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
    BinanceAPIc &Api;
    unordered_map<string, double> balance; // Общий баланс (free + locked)
    unordered_map<string, double> free_balance; // Только свободные средства
    unordered_map<string, double> locked_balance; // Заблокированные средства
    mutable mutex mutex_;
    thread stram_t;

    atomic<bool> run;

    double quote_qty;

    double order_price{0};
    double qnt_bought{0};

    void runing();

    void update_balance();

    void process_order(const json &order);

public:
    AccountManager(BinanceAPIc &api)
        : Api(api), run(false) {
    }

    bool has_sufficient_balance(const string &asset, double required) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = balance.find(asset);
        return it != balance.end() && it->second >= required;
    }

    void start();

    void stop();

    double get_balance(const std::string &asset);

    double get_free_balance(const string &asset); // Получить свободные средства
    double get_locked_balance(const string &asset); // Получить заблокированные средства
    double get_profit(const double &total_base_wallet);

    void on_order_executed(const json &order_response);
};
#endif //ACCOUNTMANAGER_H
