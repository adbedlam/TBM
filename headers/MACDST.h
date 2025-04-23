//
// Created by mniki on 23.04.2025.
//

#ifndef TRADE_BOT_MACDST_H
#define TRADE_BOT_MACDST_H

#include "strategy.h"

class DataMACD_ST : public DataSMA {
private:
    int macd_fast_;
    int macd_slow_;
    int macd_signal_;
    int ema_long_;

    deque<double> macd_line_;
    deque<double> signal_line_;
    deque<double> ema_long_values_;

    deque<double> high_prices_;
    deque<double> low_prices_;
    deque<double> close_prices_;

    deque<double> atr_values_;
    deque<double> supertrend_upper_;
    deque<double> supertrend_lower_;
    deque<bool> supertrend_directions_;

    double price{0};
    double atr_multiplier_;
    int atr_period_;
    int state_ = 0; // 0 - нет сигнала, 1 - готов к покупке, 2 - готов к продаже

    std::mutex strategy_mutex_;
    enum class LastTrade { NONE, BUY, SELL };
    LastTrade last_trade_ = LastTrade::NONE;
    std::chrono::system_clock::time_point last_trade_time_;

    double calculate_atr(int period);
    void calculate_supertrend();
    void calculate_macd(double &macd, double &signal, double &histogram);
    double calculate_ema(int period);
    bool is_supertrend_buy();
    bool is_supertrend_sell();
    void check_signal(const DataCSV &data) override;

public:
    DataMACD_ST(int macd_fast = 12, int macd_slow = 26, int macd_signal = 9,
                int ema_long = 200, double atr_multiplier = 3.0, int atr_period = 10);

    void update(DataCSV &data) override;
    void reset_trade_state();

    void get_macd(double &macd, double &signal, double &histogram) {
        calculate_macd(macd, signal, histogram);
    }
    double get_ema_long() { return calculate_ema(ema_long_); }
    int get_state() const { return state_; }
};
#endif //TRADE_BOT_MACDST_H
