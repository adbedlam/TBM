//
// Created by nikit on 26.04.2025.
//

#ifndef ABSTRACTSTRATEGY_H
#define ABSTRACTSTRATEGY_H
#include "common.hpp"

using std::string;

struct DataCSV {
    uint64_t timestamp;
    string symbol;
    double price;
    double high;
    double low;
    double volume;
};

class Indicator {
    virtual ~Indicator() = default;
    virtual void update(const DataCSV& candle) = 0;
    virtual void reset() = 0;

};

#endif //ABSTRACTSTRATEGY_H
