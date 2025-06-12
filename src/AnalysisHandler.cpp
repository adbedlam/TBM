//
// Created by nikit on 28.04.2025.
//

#include "AnalysisHandler.h"

AnalysisHandler::AnalysisHandler(const double& quant, const double& step_size, const double& notional) :
                                quantity(quant), step_size(step_size), min_notional(notional), matrix_signals(200, vector<int>(6, 0)){}

/*void AnalysisHandler::set_params(const double& rsi, const double& bb_up, const double& bb_low,
                    const double& bb_mean, const double& macd, const double& macd_signal, const double& ema,
                    const double& price){*/
void AnalysisHandler::set_signals(vector<int>& signals, int& idx) {
    matrix_signals[idx] = signals;
}



std::pair<bool, std::string> AnalysisHandler::check_signal(vector<int> signals) {

    if (is_cooldown()) {
        return {false, "COOLDOWN"};
    }

    std::pair<bool, std::string> strategy_1 = check_combined_signal(signals);

    if (strategy_1.first) {
        last_signal_time = std::chrono::system_clock::now();
        return {true, strategy_1.second};
    }


    return {false, "NO_SIGNAL"};
}


std::pair<bool, std::string> AnalysisHandler::check_combined_signal(vector<int>& infer_signals) {
    int combined_signal = 0;
    for (size_t i = 0; i < infer_signals.size(); ++i) {
        combined_signal += best_weights[i] * infer_signals[i];
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
        int combined = 0;
        for (int j = 0; j < weights.size(); ++j)
            combined += weights[j] * matrix_signals[i][j];

        // Симуляция сделок
        if (combined > 0) {
            // Покупаем по цене historical_data[i].price
            total_profit += historical_data[i];
        } else if (combined < 0) {
            total_profit -= historical_data[i];
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

void AnalysisHandler::set_historical(vector<double>& hist) {
    historical_data = hist;
}
