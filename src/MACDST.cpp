//
// Created by mniki on 23.04.2025.
//
#include "MACDST.h"

DataMACD_ST::DataMACD_ST(int macd_fast, int macd_slow, int macd_signal,
                         int ema_long, double atr_multiplier, int atr_period) :
        macd_fast_(macd_fast), macd_slow_(macd_slow), macd_signal_(macd_signal),
        ema_long_(ema_long), atr_multiplier_(atr_multiplier), atr_period_(atr_period) {}

void DataMACD_ST::update(DataCSV &data) {
    price = data.price;

    // Сохраняем все компоненты цены
    high_prices_.push_back(data.high); // Для теста используем цену как high
    low_prices_.push_back(data.low);  // Для теста используем цену как low
    close_prices_.push_back(data.price);
    history_price_.push_back(data.price);

    // Ограничиваем размер очередей
    size_t max_history = std::max({macd_slow_ + macd_signal_, ema_long_, atr_period_}) * 2;
    if (high_prices_.size() > max_history) {
        high_prices_.pop_front();
        low_prices_.pop_front();
        close_prices_.pop_front();
        history_price_.pop_front();
    }
    if (high_prices_.size() == ema_long_) {
        calculate_supertrend();
        check_signal(data);
    }

}

double DataMACD_ST::calculate_atr(int period) {
    if (high_prices_.size() < period + 1) return 0.0;

    deque<double> true_ranges;

    // Рассчитываем True Range для каждого периода
    for (size_t i = 1; i <= period; ++i) {
        double high = high_prices_[i];
        double low = low_prices_[i];
        double prev_close = close_prices_[i-1];

        double tr1 = high - low;
        double tr2 = std::abs(high - prev_close);
        double tr3 = std::abs(low - prev_close);

        true_ranges.push_back(std::max({tr1, tr2, tr3}));
    }

    // Первое значение ATR - среднее TR за период
    if (atr_values_.empty()) {
        double sum = std::accumulate(true_ranges.begin(), true_ranges.end(), 0.0);
        return sum / period;
    }

    // Последующие значения ATR по формуле сглаживания
    double prev_atr = atr_values_.back();
    double current_tr = true_ranges.back();
    return (prev_atr * (period - 1) + current_tr) / period;
}

void DataMACD_ST::calculate_supertrend() {
    if (high_prices_.size() < atr_period_ || close_prices_.size() < atr_period_) {
        supertrend_upper_.push_back(0);
        supertrend_lower_.push_back(0);
        supertrend_directions_.push_back(false);
        return;
    }

    double atr = calculate_atr(atr_period_);
    double basic_upper = (high_prices_.back() + low_prices_.back()) / 2 + atr_multiplier_ * atr;
    double basic_lower = (high_prices_.back() + low_prices_.back()) / 2 - atr_multiplier_ * atr;

    double final_upper = basic_upper;
    double final_lower = basic_lower;
    bool direction = true; // по умолчанию восходящий тренд

    if (!supertrend_directions_.empty()) {
        bool prev_direction = supertrend_directions_.back();

        if (prev_direction) { // предыдущий был восходящий
            final_lower = std::max(basic_lower, supertrend_lower_.back());
            if (close_prices_.back() <= final_lower) {
                direction = false; // смена на нисходящий
                final_upper = basic_upper;
            }
        } else { // предыдущий был нисходящий
            final_upper = std::min(basic_upper, supertrend_upper_.back());
            if (close_prices_.back() >= final_upper) {
                direction = true; // смена на восходящий
                final_lower = basic_lower;
            }
        }
    }

    // Сохраняем значения
    atr_values_.push_back(atr);
    supertrend_upper_.push_back(final_upper);
    supertrend_lower_.push_back(final_lower);
    supertrend_directions_.push_back(direction);

    // Ограничиваем размер очередей
    if (atr_values_.size() > 100) {
        atr_values_.pop_front();
        supertrend_upper_.pop_front();
        supertrend_lower_.pop_front();
        supertrend_directions_.pop_front();
    }
}

void DataMACD_ST::calculate_macd(double &macd, double &signal, double &histogram) {
    if (history_price_.size() < macd_slow_ + macd_signal_) {
        macd = signal = histogram = 0;
        return;
    }

    double fast_ema = 0;
    double slow_ema = 0;
    double fast_mult = 2.0 / (macd_fast_ + 1);
    double slow_mult = 2.0 / (macd_slow_ + 1);

    fast_ema = history_price_.back();
    slow_ema = history_price_.back();

    for (size_t i = history_price_.size() - 2, j = 0; j < macd_slow_; --i, ++j) {
        if (j < macd_fast_) {
            fast_ema = (history_price_[i] - fast_ema) * fast_mult + fast_ema;
        }
        slow_ema = (history_price_[i] - slow_ema) * slow_mult + slow_ema;
    }

    macd = fast_ema - slow_ema;

    if (macd_line_.size() < macd_signal_) {
        macd_line_.push_back(macd);
        signal = macd;
    } else {
        signal = signal_line_.back();
        signal = (macd - signal) * (2.0 / (macd_signal_ + 1)) + signal;
    }

    histogram = macd - signal;

    macd_line_.push_back(macd);
    signal_line_.push_back(signal);

    if (macd_line_.size() > macd_signal_ * 2) {
        macd_line_.pop_front();
        signal_line_.pop_front();
    }
}

double DataMACD_ST::calculate_ema(int period) {
    if (history_price_.size() < period) return 0.0;

    double multiplier = 2.0 / (period + 1);
    double ema = history_price_[0];

    for (size_t i = 1; i < history_price_.size(); ++i) {
        ema = (history_price_[i] - ema) * multiplier + ema;
    }

    return ema;
}

bool DataMACD_ST::is_supertrend_buy() {
    if (supertrend_directions_.empty()) return false;
    return supertrend_directions_.back(); // true = восходящий тренд = сигнал на покупку
}

bool DataMACD_ST::is_supertrend_sell() {
    if (supertrend_directions_.empty()) return false;
    return !supertrend_directions_.back(); // false = нисходящий тренд = сигнал на продажу
}

void DataMACD_ST::check_signal(const DataCSV &data) {
    std::lock_guard<std::mutex> lock(strategy_mutex_);
    auto now = std::chrono::system_clock::now();

    // 5 минут между сделками
    if (last_trade_time_ + std::chrono::minutes(5) > now) {
        return;
    }

    double macd, signal, histogram;
    calculate_macd(macd, signal, histogram);
    double ema_long = calculate_ema(ema_long_);

    // State machine
    if (state_ == 0 && macd < 0 && histogram > 0) {
        state_ = 1; // Готов к покупке
    }
    else if (state_ == 1 && is_supertrend_buy() &&
             price > ema_long * 1.005 && // 0.5% выше EMA
             last_trade_ != LastTrade::BUY) {
        trade_callback("MACD_BUY", price);
        last_trade_ = LastTrade::BUY;
        last_trade_time_ = now;
        state_ = 0;
    }
    else if (state_ == 0 && macd > 0 && histogram < 0) {
        state_ = 2; // Готов к продаже
    }
    else if (state_ == 2 && is_supertrend_sell() &&
             price < ema_long * 0.995 && // 0.5% ниже EMA
             last_trade_ != LastTrade::SELL) {
        trade_callback("MACD_SELL", price);
        last_trade_ = LastTrade::SELL;
        last_trade_time_ = now;
        state_ = 0;
    }
}

void DataMACD_ST::reset_trade_state() {
    std::lock_guard<std::mutex> lock(strategy_mutex_);
    last_trade_ = LastTrade::NONE;
    state_ = 0;
}