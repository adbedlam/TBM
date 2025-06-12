//
// Created by nikit on 28.04.2025.
//

#ifndef ANALYSISHANDLER_H
#define ANALYSISHANDLER_H

#include "common.hpp"


using std::vector;

class AnalysisHandler {
private:
    double quantity{0.001};
    double step_size{};
    double min_notional;

    double best_profit;
    vector<int> best_weights;
    vector<double> historical_data;
    bool buy = true;
    bool position_opened;

    double entry_price;
    double entry_quantity{0.001};
    double total_profit;
    double total_profit_percent;

    double cur_price{0};

    int rsi;
    int macd;
    int bb;
    int obv;
    int ichimoku;



    vector<vector<int>> matrix_signals;
    vector<int> weights();

    std::chrono::time_point<std::chrono::system_clock> last_signal_time;

    bool is_cooldown() const;

    double backtest(const std::vector<int>& weights);


public:
    explicit AnalysisHandler(const double& quant, const double& step_size, const double& notional);


    void set_signals(vector<int>& signals, int& idx);

    std::pair<bool, std::string> check_signal(vector<int> signals);


    std::pair<bool, std::string> check_combined_signal(vector<int>& infer_signals);



    void open_position(double price, double quantity);
    void close_position(double exit_price);

    double get_total_profit() const;
    double get_total_profit_percent() const;
    double get_entry_quantity() const;

    double get_min_quant() const;
    double get_step_size() const;

    double get_min_notional() const;

    void optimize_weights();

    void set_historical(vector<double>& hist);


};



#endif //ANALYSISHANDLER_H
