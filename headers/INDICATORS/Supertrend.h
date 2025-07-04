//
// Created by mniki on 29.04.2025.
//

#ifndef TRADE_BOT_SUPERTREND_H
#define TRADE_BOT_SUPERTREND_H

#include "AbstractStrategy.h"

class Supertrend : public Indicator{
private:
    std::deque<double> window;
    double sum_{0.0};
    double prev_close = 0.0;
    double close = 0.0;
    double high = 0.0;
    double low = 0.0;
    double upper_band = 0.0;
    double lower_band = 0.0;
    double prev_upper_band = 0.0;
    int period;
    double TR{0.0};
    double ATR{0.0};
    double prev_Supertrend{0.0};
    double prev_ATR{0.0};
    bool current_trend{};
    double sptr{0.0};
    int mult = 3;
public:
    Supertrend(int period);
    void update(const Candle &candle) override;
    void bands ();
    bool get_trend();
    double get_value() override;
};
#endif //TRADE_BOT_SUPERTREND_H
