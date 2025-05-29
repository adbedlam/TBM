#ifndef EMAINDICATOR_H
#define EMAINDICATOR_H
#include "AbstractStrategy.h"

class EMAIndicator : public Indicator {
private:
    int period;
    double multiplier;
    double ema;
    bool initialized = false;
    double price{0.0};

public:
    explicit EMAIndicator(int period);
    void update(const Candle &candle) override;

    double get_value() override;

    int get_signal() const;

};

#endif // EMAINDICATOR_H