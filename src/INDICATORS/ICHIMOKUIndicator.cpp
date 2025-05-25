//
// Created by mniki on 21.05.2025.
//
#include "INDICATORS/ICHIMOKUIndicator.h"

ICHIMOKUIndicator::ICHIMOKUIndicator(int p1, int p2, int p3)
        :period1(p1), period2(p2), period3(p3) {}

void ICHIMOKUIndicator::updateBuffer(std::deque<double>& buffer,
                                     double value,
                                     int period) {
    buffer.push_back(value);
    if (buffer.size() > static_cast<size_t>(period)) {
        buffer.pop_front();
    }
}
double ICHIMOKUIndicator::calculateLine(const std::deque<double> &highs, const std::deque<double> &lows) {

    if (highs.empty() || lows.empty()) return 0;
    double max_high = *std::max_element(highs.begin(), highs.end());
    double min_low = *std::min_element(lows.begin(), lows.end());
    return (max_high + min_low) / 2;
}

void ICHIMOKUIndicator::update(const Candle& candle) {

    updateBuffer(highs_TS, candle.high, period1);
    updateBuffer(lows_TS, candle.low, period1);
    updateBuffer(highs_KS, candle.high, period2);
    updateBuffer(lows_KS, candle.low, period2);
    updateBuffer(highs_SSB, candle.high, period3);
    updateBuffer(lows_SSB, candle.low, period3);

    TS = calculateLine(highs_TS, lows_TS);
    KS = calculateLine(highs_KS, lows_KS);
    SSA = (TS + KS) / 2;
    SSB = calculateLine(highs_SSB, lows_SSB);
}

double ICHIMOKUIndicator::getTS() const { return TS; }
double ICHIMOKUIndicator::getKS() const { return KS; }
double ICHIMOKUIndicator::getSSA() const { return SSA; }
double ICHIMOKUIndicator::getSSB() const { return SSB; }