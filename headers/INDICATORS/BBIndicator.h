//
// Created by nikit on 27.04.2025.
//

#ifndef BBINDICATOR_H
#define BBINDICATOR_H

#include "AbstractStrategy.h"

struct BBValues {
    double bb_low;
    double bb_mid;
    double bb_up;
};

class BollingerBandsIndicator : public Indicator {
private:
    int period;

    double multiplier = 2.0;

    std::deque<double> window;
    double sum_;

public:
    BollingerBandsIndicator(int period);

    void update(const Candle& candle) override;

    BBValues get_bands() const;

    double get_value() override;

};

#endif //BBINDICATOR_H
