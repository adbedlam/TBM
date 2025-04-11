#include "strategy.h"

// Private

double DataSMA::calcullate_sma(int window) {
    if (history_price_.size() < window) return 0.0;

    auto start = history_price_.end() - window;

    return std::accumulate(start, history_price_.end(),  0.0) / window;
}


void DataSMA::check_signal(const DataCSV& data) {
    if (should_buy() && trade_callback) {
        trade_callback("BUY", data.price);
    }
    else if (should_sell() && trade_callback) {
        trade_callback("SELL", data.price);
    }
}

// Public
void DataSMA::update(DataCSV& data) {
    history_price_.push_back(data.price);

    const int max_length = long_window_ * 2;
    if (history_price_.size() > max_length) {
        history_price_.pop_front();
    }

    check_signal(data);
}

bool DataSMA::should_buy() {
    if (history_price_.size() < long_window_) return false;

    double short_sma = calcullate_sma(short_window_);
    double long_sma = calcullate_sma(long_window_);

    return (short_sma > long_sma * (1 + threshold_));
}

bool DataSMA::should_sell() {
    if (history_price_.size() < long_window_) return false;

    double short_sma = calcullate_sma(short_window_);
    double long_sma = calcullate_sma(long_window_);

    return (short_sma < long_sma - (1 - threshold_));

}

void DataSMA::set_trade_callback(std::function<void(const string&, double)> callback) {
    trade_callback = callback;
}