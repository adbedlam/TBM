#include "ERBB.h"


DataEMA_RSI_BB::DataEMA_RSI_BB(int rsi, int bb, double bb_dev,
                               double overbought, double oversold,
                               int macd_fast, int macd_slow, int macd_signal):

   rsi_window_(rsi),
   bb_window_(bb),
   bb_std_dev_(bb_dev),
   overbought_level_(overbought),
   oversold_level_(oversold),
   macd_fast_(macd_fast),
   macd_slow_(macd_slow),
   macd_signal_(macd_signal) {}

void DataEMA_RSI_BB::update(DataCSV &data) {
    price = data.price;
    history_price_.push_back(data.price);

    size_t max_history = std::max({bb_window_ + rsi_window_, macd_slow_ + macd_signal_}) * 2;
    if (history_price_.size() > max_history) {
        history_price_.pop_front();
    }

    check_signal(data);
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

void DataEMA_RSI_BB::calculate_macd(double &macd, double &signal, double &histogram) {

    if (history_price_.size() < macd_slow_ + macd_signal_) {
        macd = signal = histogram = 0;
        return;
    }
    // Вычисляем EMA для быстрой и медленной линии
    double fast_ema = 0, slow_ema = 0;
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

    // Вычисляем сигнальную линию (EMA от MACD)
    if (macd_line_.size() < macd_signal_) {
        macd_line_.push_back(macd);
        signal = macd;
    } else {
        signal = signal_line_.back();
        signal = (macd - signal) * (2.0 / (macd_signal_ + 1)) + signal;
    }

    histogram = macd - signal;

    // Сохраняем значения для следующего расчета
    macd_line_.push_back(macd);
    signal_line_.push_back(signal);

    if (macd_line_.size() > macd_signal_ * 2) {
        macd_line_.pop_front();
        signal_line_.pop_front();
    }
}

void DataEMA_RSI_BB::check_signal(const DataCSV &data) {
    double rsi = calculate_rsi();
    double upper_bb, middle_bb, lower_bb;
    calculate_bollinger_bands(upper_bb, middle_bb, lower_bb);

    double macd, signal, histogram;
    calculate_macd(macd, signal, histogram);

    //длинная стратегия
    if (price < lower_bb && rsi < oversold_level_) {
        trade_callback("LONG_BUY", price);
    } else if (price > upper_bb && rsi > overbought_level_){
        trade_callback("LONG_SELL", price);
    }

    //короткая стратегия
    if (histogram > 0 && macd > signal) {
        trade_callback("SHORT_BUY", price);
    } else if (histogram < 0 && macd < signal) {
        trade_callback ("SHORT_SELL", price);
    }
}

/* bool DataEMA_RSI_BB::should_buy() {
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
*/