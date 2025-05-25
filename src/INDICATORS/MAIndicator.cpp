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