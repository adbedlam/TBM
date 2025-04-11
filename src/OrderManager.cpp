#include "OrderManager.h"

// Private
void OrderManager::process_orders() {
    auto last_ord = steady_clock::now();

    while (run) {
        auto now = steady_clock::now();

        auto elepsed_t = duration_cast<milliseconds>( now - last_ord).count();

        if (elepsed_t >= 1000 / oreder_rate) {
            std::lock_guard<mutex> lock(mutex_ord);

            if (!order_q.empty()) {
                auto order = order_q.front();
                order_q.pop();

                try {
                    json response = Api.create_order(order.symbol,order.action,"LIMIT",order.quant, order.price);
                    cout << endl;
                    cout << "Order exec: " << response.dump(2) << endl;


                    if(response.contains("status") && response["status"] == "FILLED") {
                        Acc.on_order_executed(response);
                    }

                    last_ord = now;
                }
                catch (const std::exception& e) {
                    cerr << "FAIL: " << e.what() << endl;
                }

            }
        }

        std::this_thread::sleep_for(10ms);
    }

}


// Public


void OrderManager::start() {
    run = true;
    stream_t = thread(&OrderManager::process_orders, this);
}

void OrderManager::stop() {
    run = false;
    if (stream_t.joinable()) {
        stream_t.join();
    }

}

void OrderManager::add_order(const string& action, const string& symbol, double price, double quant) {
    std::lock_guard<mutex> lock(mutex_ord);
    order_q.push({action, symbol, price, quant});
}


size_t OrderManager::queue_size()  {
    std::lock_guard<std::mutex> lock(mutex_ord);
    return order_q.size();
}