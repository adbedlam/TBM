//
// Created by mniki on 20.05.2025.
//

#include "INDICATORS/MAIndicator.h"
MAIndicator::MAIndicator(int period1, int period2) {
    this -> period1 = period1;
    this -> period2 = period2;
    sum1 = 0.0;
    sum2 = 0.0;
    MA1 = 0.0;
    MA2 = 0.0;
    cp = 0.0;
}
void MAIndicator:: update(const Candle &candle){
    cp = candle.price;
    window1.push_back(cp);
    sum1 += cp;

    if (window1.size() > period1) {
        sum1 -= window1.front();
        window1.pop_front();
    }
    if (window1.size() == period1) {
        MA1 = sum1 / period1;
    }

    window2.push_back(cp);
    sum2 += cp;

    if (window2.size() > period2) {
        sum2 -= window2.front();
        window2.pop_front();
    }
    if (window2.size() == period2) {
        MA2 = sum2 / period2;
    }
}
double MAIndicator::get_value() {
    return 0;
}

double MAIndicator::get_ma20() const {
    return MA1;
}
double MAIndicator::get_ma50() const {
    return MA2;
}


int MAIndicator::get_signal(double price) const {
    if (window1.size() < period1 || window2.size() < period2) return 0;

    const double epsilon = 1e-6;

    if (MA1 > MA2 + epsilon && price > MA2) return +1;
    if (MA1 < MA2 - epsilon && price < MA2) return -1;
    return 0;
}
