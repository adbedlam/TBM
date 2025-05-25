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

    double prev_ATR{0.0};

    double rsi{0};
    bool supertrend{0};
    double bb_up{0};
    double bb_low{0};
    double bb_mean{0};
    double min_compare{0};
    double ATR{0};

    double macd{0};
    double macd_signal{0};

    double ema200{0};

    bool buy = true;

    bool position_opened;
    double entry_price;
    double entry_quantity{0.001};
    double total_profit;
    double total_profit_percent;

    double cur_price{0};

    double prev_rsi{100.0};


    double macd_hist{0.0};
    double prev_macd_hist{0.0};
    double prev_macd_signal{0.0};
    double prev_macd{0.0};

    double bb_threshold{0.02};
    std::chrono::time_point<std::chrono::system_clock> last_signal_time;

public:
    explicit AnalysisHandler(const double& quant, const double& step_size, const double& notional);

   /* void set_params(const double& rsi, const double& bb_up, const double& bb_low,
                    const double& bb_mean, const double& macd, const double& macd_signal, const double& ema, const double& price);*/
    void set_params(const double& rsi, const double& bb_up, const double& bb_low,
                    const double& bb_mean, const double& macd, const double& macd_signal, const double& ema, const double& price);

    std::pair<bool, std::string> check_signal();


    std::pair<bool, std::string> check_RSI_bb_strategy();
    std::pair<bool, std::string> check_macd_ATR_bb_strategy();

    bool is_cooldown() const;

    void open_position(double price, double quantity);
    void close_position(double exit_price);

    double get_total_profit() const;
    double get_total_profit_percent() const;
    double get_entry_quantity() const;

    double get_min_quant() const;
    double get_step_size() const;

    double get_min_notional() const;
};



#endif //ANALYSISHANDLER_H
