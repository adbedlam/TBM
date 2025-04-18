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

    // Для короткой стратегии
    int macd_fast_;
    int macd_slow_;
    int macd_signal_;
    deque<double> macd_line_;
    deque<double> signal_line_;

    double price{0};
    // Методы для длинной стратегии
    double calculate_rsi();

    void calculate_bollinger_bands(double &upper, double &middle, double &lower);

    // Методы для корткой стратегии
    void calculate_macd(double &macd, double &signal, double &histogram);

    void check_signal(const DataCSV &data) override;

public:
    DataEMA_RSI_BB(int rsi = 14, int bb = 20, double bb_dev = 2.0,
                   double overbought = 70.0, double oversold = 30.0,
                   int macd_fast = 12, int macd_slow = 26, int macd_signal = 9);

    void update(DataCSV &data) override;

    double get_macd_mid() {
        if (macd_line_.empty()) {
            return 0;
        }
        return macd_line_.back();
    }

    double get_macd_signal() {
        if (signal_line_.empty()) {
            return 0;
        }
        return signal_line_.back();
    }

    double get_rsi() { return calculate_rsi(); }

    void get_bollinger_bands(double &upper, double &middle, double &lower) {
        calculate_bollinger_bands(upper, middle, lower);
    }
    
    void get_macd(double &macd, double &signal, double &histogram) {
        calculate_macd(macd, signal, histogram);
    }
    void Calculate_profit();
};

#endif //TRADE_BOT_E_R_BB_H
