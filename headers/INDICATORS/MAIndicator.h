//
// Created by mniki on 20.05.2025.
//

#ifndef TRADE_BOT_MAINDICATOR_H
#define TRADE_BOT_MAINDICATOR_H

#include "AbstractStrategy.h"

class MAIndicator: public Indicator{

private:
    std::deque<double> window;
    int period;
    double MA;

    double sum = 0.0;
    double cp = 0.0;

public:
    MAIndicator(int period);
    void update(const Candle &candle) override;
    double get_value() override;

    int get_signal(double price) const;

};
#endif //TRADE_BOT_MAINDICATOR_H
