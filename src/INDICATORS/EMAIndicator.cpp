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
}

double EMAIndicator::get_value() {
    return ema;
}

