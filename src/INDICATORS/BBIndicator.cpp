//
// Created by nikit on 27.04.2025.
//
#include "INDICATORS/BBIndicator.h"

BollingerBandsIndicator::BollingerBandsIndicator(int period) {
    this->period = period;
    sum_ = 0.0;
}

void BollingerBandsIndicator::update(const Candle &candle) {
    window.push_back(candle.price);
    sum_ += candle.price;

    if (window.size() > period) {
        sum_ -= window.front();
        window.pop_front();
    }
}

BBValues BollingerBandsIndicator::get_bands() const {
    if (window.empty()) {
        return {0.0, 0.0, 0.0};
    }
    if (window.size() == period) {
        double mean = sum_ / window.size();
        double variance = 0.0;
        const auto sample = window.size();

        for (const auto &price: window) {
            variance += (price - mean) * (price - mean);
        }
        variance /= (sample - 1);

        double stddev = std::sqrt(variance);

        return {mean - (multiplier * stddev), mean, mean + (multiplier * stddev)};
    }
    else
        return {0.0, 0.0, 0.0};
}
int BollingerBandsIndicator::get_signal() const{
    if (window.size() < period) return 0;

    BBValues bands = get_bands();
    double price = window.back();

    if (price < bands.bb_low) {
        return +1;
    }
    if (price > bands.bb_up) {
        return -1;
    }
    return 0;
}


double BollingerBandsIndicator::get_value() {
    return get_bands().bb_mid;
}


