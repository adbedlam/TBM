//
// Created by nikit on 28.04.2025.
//

#include "AnalysisHandler.h"

AnalysisHandler::AnalysisHandler(const double& quant, const double& step_size, const double& notional) :
                                quantity(quant), step_size(step_size), min_notional(notional){}

/*void AnalysisHandler::set_params(const double& rsi, const double& bb_up, const double& bb_low,
                    const double& bb_mean, const double& macd, const double& macd_signal, const double& ema,
                    const double& price){*/
void AnalysisHandler::set_params(int rsi_signal, int macd_signal, int bb_signal, int obv_signal, int ichimoku_signal) {
     // rsi = rsi_signal;
     // macd = macd_signal;
     // bb = bb_signal;
     // obv = obv_signal;
     // ichimoku = ichimoku_signal;
     signals = {rsi_signal, macd_signal, bb_signal, obv_signal, ichimoku_signal};
}



std::pair<bool, std::string> AnalysisHandler::check_signal() {

    if (is_cooldown()) {
        return {false, "COOLDOWN"};
    }

    std::pair<bool, std::string> strategy_1 = check_combined_signal();

    if (strategy_1.first) {
        last_signal_time = std::chrono::system_clock::now();
        return {true, strategy_1.second};
    }


    return {false, "NO_SIGNAL"};
}


std::pair<bool, std::string> AnalysisHandler::check_combined_signal() {
    int combined_signal = 0;
    for (size_t i = 0; i < signals.size(); ++i) {
        combined_signal += weights[i] * signals[i];
    }

    if (combined_signal > 0) return {true, "BUY_SIGNAL"};
    if (combined_signal < 0) return {true, "SELL_SIGNAL"};
    return {false, "NO_ACTION"};
}




void AnalysisHandler::optimize_weights() {
    std::vector<int> current_weights(5, 0);

    for (int ma = 0; ma <= 4; ++ma)
        for (int rsi = 0; rsi <= 4; ++rsi)
            for (int macd = 0; macd <= 4; ++macd)
                for (int bb = 0; bb <= 4; ++bb)
                    for (int obv = 0; obv <= 4; ++obv)
                        for (int ich = 0; ich <= 4; ++ich) {
                            current_weights = {ma, rsi, macd, bb, obv, ich};

                            double simulated_profit = backtest(current_weights);

                            if (simulated_profit > best_profit) {
                                best_profit = simulated_profit;
                                best_weights = current_weights;
                            }
                        }

    std::cout << "Best weights found: ";
    for (int w : best_weights) {
        std::cout << w << " ";
    }
}


double AnalysisHandler::backtest(const std::vector<int>& weights) {
    double total_profit = 0.0;

    for (int i = 1; i < historical_data.size(); ++i) {
        std::vector<int> signals = calculate_signals(historical_data[i]);
        int combined = 0;
        for (int j = 0; j < weights.size(); ++j)
            combined += weights[j] * signals[j];

        // Симуляция сделок
        if (combined > 0) {
            // Покупаем по цене historical_data[i].price
            total_profit += simulate_buy_sell(i);
        } else if (combined < 0) {
            total_profit -= simulate_sell_buy(i);
        }
    }

    return total_profit;
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

double AnalysisHandler::get_min_quant() const {
    return quantity;
}

double AnalysisHandler::get_step_size() const {
    return step_size;
}

double AnalysisHandler::get_min_notional() const{
    return min_notional;
}