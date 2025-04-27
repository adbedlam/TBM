//
// Created by nikit on 26.04.2025.
//

#ifndef ABSTRACTSTRATEGY_H
#define ABSTRACTSTRATEGY_H
#include "../common.hpp"

using std::string;

struct Candle {
    uint64_t timestamp;
    string symbol;
    double price;
    double high;
    double low;
    double volume;
};

class Indicator {
    virtual void initialize(const std::vector<Candle>& history) {
        for (const auto& candle : history) {
            update(candle);
        }
    }

    virtual void update(const Candle& candle) = 0;

    virtual double get_value() = 0;

    // virtual ~Indicator() = default;

};

#endif //ABSTRACTSTRATEGY_H
