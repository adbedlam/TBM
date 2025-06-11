//
// Created by mniki on 20.05.2025.
//

#ifndef TRADE_BOT_MAINDICATOR_H
#define TRADE_BOT_MAINDICATOR_H

#include "AbstractStrategy.h"

class MAIndicator: public Indicator{

private:
    std::deque<double> window1;
    std::deque<double> window2;
    int period1;
    int period2;
    double MA1;
    double MA2;

    double sum1 = 0.0;
    double sum2 = 0.0;
    double cp = 0.0;

public:
    MAIndicator(int period1 = 20, int period2 = 50);
    void update(const Candle &candle) override;
    double get_value() override;
    double get_ma20() const;
    double get_ma50() const;

    int get_signal(double price) const;

};
#endif //TRADE_BOT_MAINDICATOR_H
