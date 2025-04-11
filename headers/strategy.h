
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
    double volume;
};

class DataSMA {
private:
    deque<double> history_price_;

    int short_window_{};
    int long_window_{};

    double threshold_{};

    function<void(const string&, double)> trade_callback;

    double calcullate_sma(int window);
    void check_signal(const DataCSV& data);

public:
    DataSMA(int s_window = 20, int l_window  = 500, double thresh = 0.05) : short_window_(s_window),
                                                         long_window_(l_window),
                                                         threshold_(thresh){};


    void update(DataCSV& data);
    bool should_buy();
    bool should_sell();

    void set_trade_callback(function<void(const string&, double)> callback);


    double get_short_sma() { return calcullate_sma(short_window_); }
    double get_long_sma() { return calcullate_sma(long_window_); }

};

#endif //STRATEGY_H
