//
// Created by mniki on 29.04.2025.
//

#include "INDICATORS/Supertrend.h"
Supertrend::Supertrend(int period) {
    this -> period = period;
    high = 0.0;
    close = 0.0;
    low = 0.0;

}

void Supertrend::update(const Candle &candle){
    high = candle.high;
    low = candle.low;
    prev_close = close;
    close = candle.price;
    TR = std::max({high - low, high - prev_close, low - prev_close});

    window.push_back(TR);
    sum_ += TR;

    if (window.size() > period) {
        sum_ -= window.front();
        window.pop_front();
    }

   if (window.size() == period){
       ATR = sum_ / period;
       prev_upper_band = upper_band;
       upper_band = ((high + low) / 2) + (mult * ATR);
       lower_band = ((high + low) / 2) - (mult * ATR);
   }
}
bool Supertrend::get_trend(){
    if (close > prev_upper_band) return true;
    return false;
}
double Supertrend::get_value() {
    return ATR;
}
