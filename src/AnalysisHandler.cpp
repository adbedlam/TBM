//
// Created by nikit on 28.04.2025.
//

#include "AnalysisHandler.h"

AnalysisHandler::AnalysisHandler(const double &quant) : quantity(quant){}

/*void AnalysisHandler::set_params(const double& rsi, const double& bb_up, const double& bb_low,
                    const double& bb_mean, const double& macd, const double& macd_signal, const double& ema,
                    const double& price){*/
void AnalysisHandler::set_params(const bool& supertrend,const double& ATR, const double& bb_up, const double& bb_low,
                                 const double& bb_mean, const double& macd, const double& macd_signal, const double& ema,
                                 const double& price) {
    cur_price = price;

/*    this->prev_rsi = this->rsi;
    this->rsi = rsi;*/
    this->ATR = ATR;
    this->supertrend = supertrend;
    this->bb_up = bb_up;
    this->bb_low = bb_low;
    this->bb_mean = bb_mean;

    this->prev_macd = this->macd;
    this->macd = macd;
    this->prev_macd_signal = this->macd_signal;
    this->macd_signal = macd_signal;

    this->prev_macd_hist = this->macd_hist;

    this->macd_hist = macd - macd_signal;

    this->ema200 = ema;
}



std::pair<bool, std::string> AnalysisHandler::check_signal() {

    if (is_cooldown()) {
        return {false, "COOLDOWN"};
    }

    std::pair<bool, std::string> strategy_1 = check_macd_ATR_bb_strategy();
    std::pair<bool, std::string> strategy_2 = check_macd_ema_supertrend_strategy();

    if (strategy_1.first) {
        last_signal_time = std::chrono::system_clock::now();
        return {true, strategy_1.second};
    }

    if (strategy_2.first) {
        last_signal_time = std::chrono::system_clock::now();
        return {true, strategy_2.second};
    }

    return {false, "NO_SIGNAL"};
}


std::pair<bool, std::string>  AnalysisHandler::check_macd_ATR_bb_strategy() {

    std::list<double> price_history;

    price_history.push_back(cur_price);

    // TODO надо сделать дни
    if (cur_price <= bb_low && ATR > min_compare && prev_macd_hist < 0 && macd_hist > 0 )
        return {true, "STRATEGY_1_SELL"};

    if (cur_price >- bb_up && ATR > min_compare && prev_macd_hist > 0 && macd_hist < 0)
        return {true, "STRATEGY_1_BUY"};

    return {false, ""};
}

std::pair<bool, std::string> AnalysisHandler::check_macd_ema_supertrend_strategy() {

    int trend = 0;

    // Возрастающий тренд Бычок
    if (prev_macd_hist < 0 && macd_hist > 0)
        trend = 1;
    if (trend == 1 && cur_price > ema200 && supertrend == true)
        return {true, "STRATEGY_2_BUY"};
    if (prev_macd_hist > 0 && macd_hist < 0)
        trend = 2;
    if (trend == 2 && cur_price < ema200 && supertrend == false)
        return {true, "STRATEGY_2_SELL"};

    return {false, ""};

}


bool AnalysisHandler::is_cooldown() const {
        auto now = std::chrono::system_clock::now();
        return duration_cast<std::chrono::seconds>(now - last_signal_time).count() < 300;
}


void AnalysisHandler::open_position(double price, double quantity) {
    position_opened = true;
    entry_price = price;
    entry_quantity = quantity;
}

void AnalysisHandler::close_position(double exit_price) {
    if(position_opened) {
        double profit = (exit_price - entry_price) * entry_quantity;
        double profit_percent = ((exit_price - entry_price)/entry_price) * 100;

        total_profit += profit;
        total_profit_percent += profit_percent;

        position_opened = false;
        entry_price = 0.0;
        entry_quantity = 0.0;
    }
}

double AnalysisHandler::get_total_profit() const {
    return total_profit;
}

double AnalysisHandler::get_total_profit_percent() const {
    return total_profit_percent;
}

double AnalysisHandler::get_entry_quantity() const {
    return entry_quantity;
}