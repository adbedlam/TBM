//
// Created by nikit on 26.04.2025.
//

#ifndef RSIINDICATOR_H
#define RSIINDICATOR_H

#include "AbstractStrategy.h"

class RSIIndicator : public Indicator {
private:
    int period;

    double prev_price = 0.0;
    double mean_gain = 0.0;
    double mean_loss = 0.0;

    int count = 0;
    bool initialized;

public:
    RSIIndicator(int period);
    void update(const Candle &candle) override;
    double get_value() override;

    int get_signal() const;


};


#endif //RSIINDICATOR_H
