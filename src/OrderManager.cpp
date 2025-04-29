#include "OrderManager.h"



// Private
void OrderManager::process_orders() {
    auto last_ord = steady_clock::now();

    while (run) {
        auto now = steady_clock::now();

        auto elepsed_t = duration_cast<milliseconds>(now - last_ord).count();

        if (elepsed_t >= 1000 / oreder_rate) {
            std::lock_guard<mutex> lock(mutex_ord);

            if (!order_q.empty()) {
                auto order = order_q.front();
                order_q.pop();

                try {
                    json response = Api.create_order(order.symbol, order.action, "LIMIT", order.quant, order.price);
                    cout << endl;
                    cout << "Order exec: " << endl; //<< response.dump(2) << endl;


                    if (response.contains("status") && response["status"] == "FILLED") {
                        Acc.on_order_executed(response);
                    }

                    uint64_t timestamp = response["transactTime"].get<uint64_t>();


                    double commission = 0.0;
                    if (!response["fills"].empty()) {
                        cout << "Make order";
                        auto& fill = response["fills"][0];
                        if (fill.contains("commission") && !fill["commission"].is_null()) {
                            if (fill["commission"].is_string()) {
                                commission = stod(fill["commission"].get<std::string>());
                            } else if (fill["commission"].is_number()) {
                                commission = fill["commission"].get<double>();
                            }
                        }
                    }

                    logger.log_data(timestamp, order.action, order.symbol, order.quant, order.price, commission);



                    last_ord = now;
                } catch (const std::exception &e) {
                    cerr << "FAIL: " << e.what() << endl;
                }
            } //          << order.price << ","

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

void OrderManager::add_order(const string &action, const string &symbol, double price, double quant) {
    std::lock_guard<mutex> lock(mutex_ord);
    cout << action << " " << quant << endl;
    order_q.push({action, symbol, price, quant});
}


size_t OrderManager::queue_size() {
    std::lock_guard<std::mutex> lock(mutex_ord);
    return order_q.size();
}
