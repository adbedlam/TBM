//
// Created by nikit on 28.04.2025.
//

#include "AnalysisHandler.h"

AnalysisHandler::AnalysisHandler(const double &quant) : quantity(quant){}

void AnalysisHandler::set_params(const double& rsi, const double& bb_up, const double& bb_low,
                    const double& bb_mean, const double& macd, const double& macd_signal, const double& ema,
                    const double& price){

    cur_price = price;

    this->prev_rsi = this->rsi;
    this->rsi = rsi;

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

    std::pair<bool, std::string> long_term_signal = check_long_term_strategy();
    std::pair<bool, std::string> short_term_signal = check_short_term_strategy();

    if (long_term_signal.first) {
        last_signal_time = std::chrono::system_clock::now();
        return {true, long_term_signal.second};
    }

    if (short_term_signal.first) {
        last_signal_time = std::chrono::system_clock::now();
        return {true, short_term_signal.second};
    }

    return {false, "NO_SIGNAL"};
}


std::pair<bool, std::string>  AnalysisHandler::check_long_term_strategy() {

    std::list<double> price_history;

    price_history.push_back(cur_price);

    // TODO надо сделать дни
    // Возрастающий тренд Бычок
    if (cur_price > ema200 && price_history.size() >= 14 * 96) {
        bool trend_up = std::all_of(price_history.begin(), price_history.end(),
            [this](double p) { return p > ema200; });

        bool near_bb_low = cur_price <= bb_low * 1.01;
        bool rsi_status = rsi >= 40 && rsi <= 50;
        bool macd_cross = macd > macd_signal && prev_macd_hist <= 0;

        if (trend_up && near_bb_low && rsi_status && macd_cross) {
            return {true, "LONG_BUY"};
        }
    }

    // Убывающий тренд Мишка
    if (cur_price < ema200 && price_history.size() >= 14) {
        bool trend_down = std::all_of(price_history.begin(), price_history.end(),
            [this](double p) { return p < ema200; });

        bool near_bb_high = cur_price >= bb_up * 0.99;
        bool rsi_status = rsi >= 50 && rsi <= 60;
        bool macd_cross = macd < macd_signal && prev_macd_hist >= 0;

        if (trend_down && near_bb_high && rsi_status && macd_cross) {
            return {true, "LONG_SELL"};
        }
    }

    return {false, ""};

}

std::pair<bool, std::string> AnalysisHandler::check_short_term_strategy() {
    double bb_width = bb_up - bb_low;
    bool bb_squeeze = (bb_width / cur_price) < bb_threshold;

    // Возрастающий тренд Бычок
    if (bb_squeeze && cur_price > ema200) {
        bool rsi_bullish = (prev_rsi < 30 && rsi >= 30);
        bool macd_bullish = (macd_hist > 0 && prev_macd_hist <= 0);
        if (rsi_bullish && macd_bullish) return {true, "SHORT_BUY"};
    }

    // Убывающий тренд Мишка
    if (bb_squeeze && cur_price < ema200) {
        bool rsi_bearish = (prev_rsi > 70 && rsi <= 70);
        bool macd_bearish = (macd_hist < 0 && prev_macd_hist >= 0);
        if (rsi_bearish && macd_bearish) return {true, "SHORT_SELL"};
    }

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