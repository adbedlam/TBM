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

    double rsi{0};

    double bb_up{0};
    double bb_low{0};
    double bb_mean{0};

    double macd{0};
    double macd_signal{0};

    double ema200{0};

    bool buy = true;


    double cur_price{0};

    double prev_rsi{100.0};


    double macd_hist{0.0};
    double prev_macd_hist{0.0};
    double prev_macd_signal{0.0};
    double prev_macd{0.0};

    double bb_threshold{0.02};
    std::chrono::time_point<std::chrono::system_clock> last_signal_time;

public:
    explicit AnalysisHandler(const double& quant);

    void set_params(const double& rsi, const double& bb_up, const double& bb_low,
                    const double& bb_mean, const double& macd, const double& macd_signal, const double& ema, const double& price);

    std::pair<bool, std::string> check_signal();


    std::pair<bool, std::string> check_long_term_strategy();
    std::pair<bool, std::string> check_short_term_strategy();

    bool is_cooldown() const;


};



#endif //ANALYSISHANDLER_H
