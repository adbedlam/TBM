#ifndef STRATEGY_H
#define STRATEGY_H
#include "common.hpp"

using std::string;
using std::deque;
using std::function;

struct DataCSV {
    uint64_t timestamp;
    string symbol;
    double price;
    double high;
    double low;
    double volume;
};

class DataSMA {
protected:
    deque<double> history_price_;
    function<void(const string &, double)> trade_callback;
    int short_window_;
    int long_window_;
    double threshold_;

    virtual double calculate_sma(int window);

    virtual void check_signal(const DataCSV &data);

public:
    DataSMA(int s_window = 20, int l_window = 500, double thresh = 0.05)
        : short_window_(s_window), long_window_(l_window), threshold_(thresh) {
    }

    virtual ~DataSMA() = default;

    virtual void update(DataCSV &data);

    virtual bool should_buy();

    virtual bool should_sell();

    virtual void set_trade_callback(function<void(const string &, double)> callback);

    virtual double get_short_sma() { return calculate_sma(short_window_); }
    virtual double get_long_sma() { return calculate_sma(long_window_); }
};

#endif //STRATEGY_H
