//
// Created by mniki on 20.05.2025.
//

#include "INDICATORS/MAIndicator.h"
MAIndicator::MAIndicator(int period) {
    this -> period = period;
    sum = 0.0;
    cp = 0.0;
}
void MAIndicator:: update(const Candle &candle){
    cp = candle.price;
    window.push_back(cp);
    sum += cp;

    if (window.size() > period) {
        sum -= window.front();
        window.pop_front();
    }
    if (window.size() == period) {
        MA = sum / period;
    }
}
double MAIndicator:: get_value(){
    return MA;
}

int MAIndicator::get_signal(double price) const {
    if (window.size() < period) return 0;

    const double epsilon = 1e-6;

    if (price > MA + epsilon) return +1;
    if (price < MA - epsilon) return -1;
    return 0;
}
