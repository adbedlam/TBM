#include "AccountManager.h"

void AccountManager::runing() {
    while(run) {
        try {
            update_balance();
            std::this_thread::sleep_for(std::chrono::seconds(15));
        }
        catch (const std::exception& e) {
            std::cerr << "Fail: " << e.what() << endl;
        }
    }
}

void AccountManager::update_balance() {
    lock_guard<mutex> lock(mutex_);
    try {
        std::cout << "Sending account info request..." << endl;
        json acc = Api.http_request("GET", "api/v3/account");
        std::cout << "Full API response: " << acc.dump(2) << endl;
        unordered_map<string, double> new_balance;
        unordered_map<string, double> new_free_balance;
        unordered_map<string, double> new_locked_balance;

        for (const auto& i : acc["balances"]) {
            const string asset = i["asset"];
            const double free = stod(i["free"].get<string>());
            const double locked = stod(i["locked"].get<string>());
            new_balance[asset] = free + locked;
            new_free_balance[asset] = free;
            new_locked_balance[asset] = locked;
            std:: cout << "Asset: " << asset << " Free: " << free << endl;
        }

        balance.swap(new_balance);
        free_balance.swap(new_free_balance);
        locked_balance.swap(new_locked_balance);
    }
    catch (const std::exception& e) {
        std::cerr << "Fail to update balance: " << e.what() << endl;
    }
}
// Новый метод для получения свободных средств
double AccountManager::get_free_balance(const string& asset) {
    lock_guard<mutex> lock(mutex_);
    auto it = free_balance.find(asset);
    return it != free_balance.end() ? it->second : 0.0;
}

double AccountManager::get_locked_balance(const string& asset) {
    lock_guard<mutex> lock(mutex_);
    auto it = locked_balance.find(asset);
    return it != locked_balance.end() ? it->second : 0.0;
}
void AccountManager::process_order(const json& order) {
    try {
        const std::string side = order["side"];
        const double quote_qty = std::stod(order["cummulativeQuoteQty"].get<std::string>());
        double commission = 0.0;

        for(const auto& fill : order["fills"]) {
            commission += std::stod(fill["commission"].get<std::string>());
        }

        if(side == "BUY") {
            profit -= (quote_qty + commission);
        } else {
            profit += (quote_qty - commission);
        }
    }
    catch(const std::exception& e) {
        std::cerr << "Fail to process order: " << e.what() << endl;
    }
}

// Public

void AccountManager::start() {

    if (!run) {
        run = true;
        update_balance();
        stram_t = thread(&AccountManager::run, this);
    }
}

void AccountManager::stop() {

    run = false;
    if(stram_t.joinable()) {
        stram_t.join();
    }

}

double AccountManager::get_balance(const std::string& asset) {

    std::lock_guard<std::mutex> lock(mutex_);
    auto it = balance.find(asset);
    return it != balance.end() ? it->second : 0.0; // Робот
}

double AccountManager::get_profit() {
    return profit;
}

void AccountManager::on_order_executed(const json& order_response) {
    std::lock_guard<std::mutex> lock(mutex_);
    process_order(order_response);
    update_balance();
}
