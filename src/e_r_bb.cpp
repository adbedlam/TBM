#include "e_r_bb.h"
#include <numeric>
#include <cmath>

DataEMA_RSI_BB::DataEMA_RSI_BB(int ema_short, int ema_long, int rsi,
                               int bb, double bb_dev,
                               double overbought, double oversold) :
        ema_short_window_(ema_short),
        ema_long_window_(ema_long),
        rsi_window_(rsi),
        bb_window_(bb),
        bb_std_dev_(bb_dev),
        overbought_level_(overbought),
        oversold_level_(oversold) {}

void DataEMA_RSI_BB::update(DataCSV& data) {
    history_price_.push_back(data.price);
    const int max_length = std::max({ema_long_window_ * 2, rsi_window_ * 2, bb_window_ * 2});
    if (history_price_.size() > max_length) {
        history_price_.pop_front();
    }
    check_signal(data);
}

void DataEMA_RSI_BB::check_signal(const DataCSV& data) {
    if (this->should_buy() && this->trade_callback) {
        this->trade_callback("BUY", data.price);
    }
    else if (this->should_sell() && this->trade_callback) {
        this->trade_callback("SELL", data.price);
    }
}

double DataEMA_RSI_BB::calculate_ema(int window) {
    if (history_price_.size() < window) return 0.0;

    double ema = std::accumulate(history_price_.end() - window, history_price_.end(), 0.0) / window;
    double multiplier = 2.0 / (window + 1.0);

    for (auto it = history_price_.end() - window; it != history_price_.end(); ++it) {
        ema = (*it - ema) * multiplier + ema;
    }

    return ema;
}

double DataEMA_RSI_BB::calculate_rsi() {
    if (history_price_.size() <= rsi_window_) return 50.0;

    std::vector<double> gains, losses;

    for (size_t i = history_price_.size() - rsi_window_; i < history_price_.size() - 1; ++i) {
        double change = history_price_[i+1] - history_price_[i];
        if (change > 0) {
            gains.push_back(change);
            losses.push_back(0.0);
        } else {
            gains.push_back(0.0);
            losses.push_back(-change);
        }
    }

    double avg_gain = std::accumulate(gains.begin(), gains.end(), 0.0) / rsi_window_;
    double avg_loss = std::accumulate(losses.begin(), losses.end(), 0.0) / rsi_window_;

    if (avg_loss == 0.0) return 100.0;

    double rs = avg_gain / avg_loss;
    return 100.0 - (100.0 / (1.0 + rs));
}

void DataEMA_RSI_BB::calculate_bollinger_bands(double& upper, double& middle, double& lower) {
    if (history_price_.size() < bb_window_) {
        upper = middle = lower = 0.0;
        return;
    }

    middle = std::accumulate(history_price_.end() - bb_window_, history_price_.end(), 0.0) / bb_window_;
    double sum_sq = 0.0;

    for (auto it = history_price_.end() - bb_window_; it != history_price_.end(); ++it) {
        sum_sq += std::pow(*it - middle, 2);
    }

    double std_dev = std::sqrt(sum_sq / bb_window_);
    upper = middle + bb_std_dev_ * std_dev;
    lower = middle - bb_std_dev_ * std_dev;
}

bool DataEMA_RSI_BB::should_buy() {
    if (history_price_.size() < ema_long_window_) return false;

    double ema_short = calculate_ema(ema_short_window_);
    double ema_long = calculate_ema(ema_long_window_);
    double rsi = calculate_rsi();
    double upper_bb, middle_bb, lower_bb;
    calculate_bollinger_bands(upper_bb, middle_bb, lower_bb);

    double current_price = history_price_.back();

    bool ema_condition = ema_short > ema_long;
    bool rsi_condition = rsi < oversold_level_;
    bool bb_condition = current_price < lower_bb;

    return ema_condition && rsi_condition && bb_condition;
}

bool DataEMA_RSI_BB::should_sell() {
    if (history_price_.size() < ema_long_window_) return false;

    double ema_short = calculate_ema(ema_short_window_);
    double ema_long = calculate_ema(ema_long_window_);
    double rsi = calculate_rsi();
    double upper_bb, middle_bb, lower_bb;
    calculate_bollinger_bands(upper_bb, middle_bb, lower_bb);

    double current_price = history_price_.back();

    bool ema_condition = ema_short < ema_long;
    bool rsi_condition = rsi > overbought_level_;
    bool bb_condition = current_price > upper_bb;

    return ema_condition || rsi_condition || bb_condition;
}