#include "ERBB.h"


DataEMA_RSI_BB::DataEMA_RSI_BB(int rsi, int bb, double bb_dev,
                               double overbought, double oversold):

   rsi_window_(rsi),
   bb_window_(bb),
   bb_std_dev_(bb_dev),
   overbought_level_(overbought),
   oversold_level_(oversold) {}

void DataEMA_RSI_BB::update(DataCSV &data) {
    price = data.price;
    history_price_.push_back(data.price);

    size_t max_history = bb_window_ + rsi_window_;
    if (history_price_.size() > max_history) {
        history_price_.pop_front();
    }

    check_signal(data);
}
void DataEMA_RSI_BB::check_signal(const DataCSV &data) {
    std::lock_guard<std::mutex> lock(strategy_mutex_);
    auto now = std::chrono::system_clock::now();

    if (last_trade_time_ + std::chrono::minutes(5) > now) {
        return;
    }

    double rsi = calculate_rsi();
    double upper_bb, middle_bb, lower_bb;
    calculate_bollinger_bands(upper_bb, middle_bb, lower_bb);

    if (price < lower_bb && rsi < oversold_level_ &&
        last_trade_ != LastTrade::LONG_BUY) {
        trade_callback("LONG_BUY", price);
        last_trade_ = LastTrade::LONG_BUY;
        last_trade_time_ = now;
    }
    else if (price > upper_bb && rsi > overbought_level_ &&
             last_trade_ != LastTrade::LONG_SELL) {
        trade_callback("LONG_SELL", price);
        last_trade_ = LastTrade::LONG_SELL;
        last_trade_time_ = now;
    }
}


void DataEMA_RSI_BB::reset_trade_state() {
    std::lock_guard<std::mutex> lock(strategy_mutex_);
    last_trade_ = LastTrade::NONE;
}

void DataEMA_RSI_BB::Calculate_profit() {
}

double DataEMA_RSI_BB::calculate_rsi() {
    if (history_price_.size() < rsi_window_ + 1) return 50.0;

    std::deque<double> gains, losses;

    for (size_t i = 1; i < rsi_window_ + 1; ++i) {
        double diff = history_price_[i] - history_price_[i - 1];
        if (diff > 0) {
            gains.push_back(diff);
            losses.push_back(0.0);
        } else {
            gains.push_back(0.0);
            losses.push_back(-diff);
        }
    }

    double avg_gain = std::accumulate(gains.begin(), gains.end(), 0.0) / rsi_window_;
    double avg_loss = std::accumulate(losses.begin(), losses.end(), 0.0) / rsi_window_;

    if (avg_loss == 0.0) return 100.0;

    double rs = avg_gain / avg_loss;
    return 100.0 - (100.0 / (1.0 + rs));
}

void DataEMA_RSI_BB::calculate_bollinger_bands(double &upper, double &middle, double &lower) {
    if (history_price_.size() < bb_window_) {
        upper = middle = lower = 0.0;
        return;
    }

    deque<double> recent_prices(history_price_.end() - bb_window_, history_price_.end());
    double sum = accumulate(recent_prices.begin(), recent_prices.end(), 0.0);
    middle = sum / bb_window_;

    double variance = 0.0;
    for (double p : recent_prices) {
        variance += pow(p - middle, 2);
    }
    double stddev = sqrt(variance / bb_window_);

    upper = middle + bb_std_dev_ * stddev;
    lower = middle - bb_std_dev_ * stddev;
}



