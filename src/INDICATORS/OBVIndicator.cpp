//
// Created by mniki on 21.05.2025.
//
#include "INDICATORS/OBVIndicator.h"

OBVIndicator::OBVIndicator(int period) {
    this -> period = period;
}
void OBVIndicator:: update(const Candle &candle){

    volume = candle.volume;
    prev_close = close;
    close = candle.price;
    prev_OBV = OBV;

    if (prev_close == 0.0)
        OBV = volume;

    else {
        if (close > prev_close)
            OBV = prev_OBV + volume;

        if (close < prev_close)
            OBV = prev_OBV - volume;

        else
            OBV = prev_OBV;
    }
}

double OBVIndicator::get_value() {
    return OBV;
}

int OBVIndicator::get_signal() const {
    const double epsilon = 1e-6;

    if (OBV > prev_OBV + epsilon) return +1;
    if (OBV < prev_OBV - epsilon) return -1;
    return 0;
}
