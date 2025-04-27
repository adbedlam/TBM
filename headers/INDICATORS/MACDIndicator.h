//
// Created by nikit on 27.04.2025.
//

#ifndef MACDINDICATOR_H
#define MACDINDICATOR_H

#include "AbstractStrategy.h"
#include "EMAIndicator.h"

struct MACD_values {
    double macd;
    double signal;
};

class MACDIndicator : public Indicator {
private:
    EMAIndicator short_ema;
    EMAIndicator long_ema;
    EMAIndicator signal_ema;
    double macd_line;

public:
    MACDIndicator(int short_period, int long_period, int signal_period);
    void update(const Candle& candle) override;

    MACD_values get_macd() ;
    double get_value() override;

};

#endif //MACDINDICATOR_H
