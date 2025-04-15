//
// Created by mniki on 15.04.2025.
//

#ifndef TRADE_BOT_E_R_BB_H
#define TRADE_BOT_E_R_BB_H

#include "strategy.h"
#include <vector>
#include <cmath>

class DataEMA_RSI_BB : public DataSMA {
private:
    int ema_short_window_;
    int ema_long_window_;
    int rsi_window_;
    int bb_window_;
    double bb_std_dev_;
    double overbought_level_;
    double oversold_level_;

    double calculate_ema(int window);
    double calculate_rsi();
    void calculate_bollinger_bands(double& upper, double& middle, double& lower);
    void check_signal(const DataCSV& data) override;

public:
    DataEMA_RSI_BB(int ema_short = 9, int ema_long = 21, int rsi = 14,
                   int bb = 20, double bb_dev = 2.0,
                   double overbought = 70.0, double oversold = 30.0);

    void update(DataCSV& data) override;
    bool should_buy() override;
    bool should_sell() override;

    // Дополнительные методы
    double get_short_ema() { return calculate_ema(ema_short_window_); }
    double get_long_ema() { return calculate_ema(ema_long_window_); }
    double get_rsi() { return calculate_rsi(); }
    void get_bollinger_bands(double& upper, double& middle, double& lower) {
        calculate_bollinger_bands(upper, middle, lower);
    }
};

#endif //TRADE_BOT_E_R_BB_H
