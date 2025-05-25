//
// Created by mniki on 20.05.2025.
//

#ifndef TRADE_BOT_OBVINDICATOR_H
#define TRADE_BOT_OBVINDICATOR_H

#include "AbstractStrategy.h"

class OBVIndicator: public Indicator{

private:
    int period;
    double prev_OBV = 0.0;
    double OBV = 0.0;
    double close = 0.0;
    double prev_close = 0.0;
    double volume = 0.0;
public:
    OBVIndicator(int period);
    void update(const Candle &candle) override;
    double get_value() override;
};
#endif //TRADE_BOT_OBVINDICATOR_H
