#include "AccountManager.h"

void AccountManager::runing() {
    while (run) {
        try {
            update_balance();
            std::this_thread::sleep_for(std::chrono::seconds(3));
        } catch (const std::exception &e) {
            std::cerr << "Fail: " << e.what() << endl;
        }
    }
}

void AccountManager::update_balance() {
    lock_guard<mutex> lock(mutex_);
    try {
        json acc = Api.http_request("GET", "/api/v3/account");
        unordered_map<string, double> new_balance;

        for (const auto &i: acc["balances"]) {
            const string asset = i["asset"];
            const double free = stod(i["free"].get<string>());
            const double locked = stod(i["locked"].get<string>());
            new_balance[asset] = free + locked;
        }

        balance.swap(new_balance);
    } catch (const std::exception &e) {
        std::cerr << "Fail to update balance: " << e.what() << endl;
    }
}


void AccountManager::process_order(const json &order) {
    try {
        const std::string side = order["side"];
        quote_qty = std::stod(order["cummulativeQuoteQty"].get<std::string>());
        double commission = 0.0;
        std::cout << order.dump(2) << "\n\n";
        return;
        order_price = std::stod(order["price"].get<std::string>());


        for (const auto &fill: order["fills"]) {
            commission += std::stod(fill["commission"].get<std::string>());
        }
    } catch (const std::exception &e) {
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
    if (stram_t.joinable()) {
        stram_t.join();
    }
}

double AccountManager::get_balance(const std::string &asset) {
    auto it = balance.find(asset);
    return it != balance.end() ? it->second : 0.0; // Робот
}

double AccountManager::get_profit(const double &total_base_wallet) {
    double current_wallet = get_balance("USDT") + get_balance("BTC") * order_price;

    if (order_price == 0) return 0;

    if ((current_wallet - total_base_wallet) > 0) {
        return current_wallet - total_base_wallet;
    } else {
        return total_base_wallet - current_wallet;
    }
}

void AccountManager::on_order_executed(const json &order_response) {
    // std::lock_guard<std::mutex> lock(mutex_);
    process_order(order_response);
    update_balance();
}
