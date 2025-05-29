//
// Created by nikit on 27.04.2025.
//
#include "INDICATORS/MACDIndicator.h"

MACDIndicator::MACDIndicator(int shortPeriod, int longPeriod, int signalPeriod) : short_ema(EMAIndicator(shortPeriod)),
                                                                                    long_ema(EMAIndicator(longPeriod)),
                                                                                    signal_ema(EMAIndicator(signalPeriod)),
                                                                                    macd_line(0.0){}

void MACDIndicator::update(const Candle &candle) {
    short_ema.update(candle);
    long_ema.update(candle);

    macd_line = short_ema.get_value() - long_ema.get_value();

    Candle macd_candle{candle.timestamp, candle.symbol, macd_line,0, 0, 0};

    signal_ema.update(macd_candle);
}

MACD_values MACDIndicator::get_macd()  {
    return {macd_line, signal_ema.get_value()};
}

double MACDIndicator::get_value() {
    return macd_line;
}

int MACDIndicator::get_signal()  {
    double signal = signal_ema.get_value();
    const double epsilon = 1e-6;

    if (macd_line > signal + epsilon) return +1;    // Buy
    if (macd_line < signal - epsilon) return -1;    // Sell
    return 0;                                       // Hold
}
