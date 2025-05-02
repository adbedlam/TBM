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
    TR = std::max({high - low, std::abs(high - prev_close), std::abs(low - prev_close)});

    window.push_back(TR);
    sum_ += TR;


    if (window.size() > period) {
        sum_ -= window.front();
        window.pop_front();
    }

   if (window.size() == period){
       ATR = sum_ / period;
       prev_Supertrend = supertrend;
       upper_band = ((high + low) / 2) + (mult * ATR);
       lower_band = ((high + low) / 2) - (mult * ATR);
   }

}
bool Supertrend::get_trend(){
    if (prev_Supertrend == 0.0){
        if(close > lower_band) {
            supertrend = lower_band;
            return true;
        }
        else{
            supertrend = upper_band;
            return false;
        }
    }
    else{
        if(close > prev_Supertrend){
            supertrend = lower_band;
            return true;
        }
        else{
            supertrend = upper_band;
            return false;
        }
    }
}

double Supertrend::get_value() {
    return ATR;
}
