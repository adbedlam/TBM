

#ifndef DATALOG_H
#define DATALOG_H
#include "common.hpp"

using nlohmann::json;
using std::ofstream;

inline void log_data(const DataCSV& data) {
    ofstream file("../utils/market_data.csv", std::ios::app);
    if (file.is_open()) {

        file << data.timestamp << ","   // Время
             << data.symbol << ","   // Символ
             << data.price << ","   // Текущая цена
             << data.volume << "\n"; // Объем
        file.close();
    }
}

#endif //DATALOG_H
