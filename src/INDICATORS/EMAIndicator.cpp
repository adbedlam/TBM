#include "INDICATORS/EMAIndicator.h"

EMAIndicator::EMAIndicator(int per)
    : period(per), multiplier(2.0 / (per + 1)), ema(0.0) {}



void EMAIndicator::update(const Candle &candle) {
    if (!initialized) {
        ema =candle.price;
        initialized = true;
    } else {
        ema = (candle.price - ema) * multiplier + ema;
    }
    price = candle.price;
}

double EMAIndicator::get_value() {
    return ema;
}

int EMAIndicator::get_signal() const {
    if (!initialized) return 0;

    const double epsilon = 1e-6;
    if (price > ema + epsilon) return +1;
    if (price < ema - epsilon) return -1;
    return 0;
}
