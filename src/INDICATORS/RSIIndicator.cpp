//
// Created by nikit on 27.04.2025.
//

#include "INDICATORS/RSIIndicator.h"

RSIIndicator::RSIIndicator(int period) {
    this->period = period;
    prev_price = 0.0;
    mean_gain = 0.0;
    mean_loss = 0.0;
    count = 0;
    initialized = false;
}


void RSIIndicator::update(const Candle &candle) {
    if (count == 0) {
        prev_price = candle.price;
        count++;
        return;
    }

    double change = candle.price - prev_price;

    double gain = std::max(change, 0.0);
    double loss = std::max(-change, 0.0);

    if (!initialized) {
        mean_gain += gain;
        mean_loss += loss;
        count++;
        if (count > period) {
            mean_gain /= period;
            mean_loss /= period;
            initialized = true;
        }
    }
    else {
        mean_gain = (mean_gain * (period - 1) + gain) / period;
        mean_loss = (mean_loss * (period - 1) + loss) / period;
    }
    prev_price = candle.price;
}


double RSIIndicator::get_value() {
    if (!initialized || mean_loss == 0.0) {
        return 100;
    }
    double rs = mean_gain/mean_loss;

    return 100.0 - (100.0 / (1 + rs));
}

int RSIIndicator::get_signal() const {
    if (!initialized) return 0;

    double rsi = 0.0;
    if (mean_loss == 0.0) {
        rsi = 100.0;
    } else {
        double rs = mean_gain / mean_loss;
        rsi = 100.0 - (100.0 / (1 + rs));
    }

    if (rsi < 30.0) return +1;
    if (rsi > 70.0) return -1;
    return 0;
}
