

#ifndef DATALOG_H
#define DATALOG_H
#include "common.hpp"

using nlohmann::json;
using std::ofstream;

inline void log_data(const DataCSV &data) {
    ofstream file("../utils/market_data.csv", std::ios::app);
    if (file.is_open()) {
        file << data.timestamp << "," // Время
                << data.symbol << "," // Символ
                << data.price << "," // Текущая цена
                << data.volume << "\n"; // Объем
        file.close();
    }
}

#endif //DATALOG_H

inline void log_indicator_data(const double& sema, const double& lema, const double& rsi,
                               const double& Bbands_u, const double& Bbands_l,
                               const double& Bbands_m, const double& price, uint64_t& time)
{
    ofstream file("../utils/indicators_metrics.csv", std::ios::app);
    if (file.is_open()) {
        file << sema << ","
             << lema << ","
             << rsi << ","
             << Bbands_u << ","
             << Bbands_l << ","
             << Bbands_m << ","
             << price << ","
             << time << "\n";


        file.close();
    }
}
