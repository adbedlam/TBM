//
// Created by mniki on 20.05.2025.
//

#ifndef TRADE_BOT_ICHIMOKUINDICATOR_H
#define TRADE_BOT_ICHIMOKUINDICATOR_H
#include "AbstractStrategy.h"
class ICHIMOKUIndicator: public Indicator{

private:
    int period1;
    int period2;
    int period3;
    double TS = 0.0;
    double KS = 0.0;
    double SSA = 0.0;
    double SSB = 0.0;

    std::deque<double> highs_TS, lows_TS;
    std::deque<double> highs_KS, lows_KS;
    std::deque<double> highs_SSB, lows_SSB;

    void updateBuffer(std::deque<double>& buffer, double value, int period);

    double calculateLine(const std::deque<double>& hihgs, const std::deque<double>& lows);

public:
    ICHIMOKUIndicator(int p1 = 9, int p2 = 26, int p3 = 52);
    void update(const Candle &candle) override;

    int get_signal() const;

    double get_value() override;

    double getTS() const;
    double getKS() const;
    double getSSA() const;
    double getSSB() const;
};
#endif //TRADE_BOT_ICHIMOKUINDICATOR_H
