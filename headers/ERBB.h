#ifndef TRADE_BOT_E_R_BB_H
#define TRADE_BOT_E_R_BB_H

#include "strategy.h"


class DataEMA_RSI_BB : public DataSMA {
private:
    // Для длинной стратегии
    int rsi_window_;
    int bb_window_;
    double bb_std_dev_;
    double overbought_level_;
    double oversold_level_;

    double price{0};

    std::mutex strategy_mutex_;
    enum class LastTrade { NONE, LONG_BUY, LONG_SELL};
    LastTrade last_trade_ = LastTrade::NONE;
    std::chrono::system_clock::time_point last_trade_time_;

    // Методы для длинной стратегии
    double calculate_rsi();

    void calculate_bollinger_bands(double &upper, double &middle, double &lower);

    void check_signal(const DataCSV &data) override;

public:
    DataEMA_RSI_BB(int rsi = 14, int bb = 20, double bb_dev = 2.0,
                   double overbought = 70.0, double oversold = 30.0);

    void update(DataCSV &data) override;
    void reset_trade_state();


    double get_rsi() { return calculate_rsi(); }

    void get_bollinger_bands(double &upper, double &middle, double &lower) {
        calculate_bollinger_bands(upper, middle, lower);
    }

    void Calculate_profit();
};

#endif //TRADE_BOT_E_R_BB_H
