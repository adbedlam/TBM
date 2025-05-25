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

    if (prev_close != 0) {
        TR = std::max({high - low, std::abs(high - prev_close), std::abs(low - prev_close)});
        window.push_back(TR);
        sum_ += TR;

        if (window.size() > period) {
            sum_ -= window.front();
            window.pop_front();
        }

        if (window.size() == period){
            prev_ATR = ATR;
            ATR = sum_ / period;
            upper_band = ((high + low) / 2) + (mult * ATR);
            lower_band = ((high + low) / 2) - (mult * ATR);

            double new_sptr;

            if (prev_Supertrend == 0.0) {
                new_sptr = (close > lower_band) ? lower_band : upper_band;
            }
            else {
                new_sptr = (close > prev_Supertrend) ? lower_band : upper_band;
            }

            current_trend = (close > new_sptr);

            sptr = new_sptr;
            prev_Supertrend = new_sptr;
        }
    }
}
bool Supertrend::get_trend(){

    return current_trend;
}

double Supertrend::get_value() {
    return ATR;
}
