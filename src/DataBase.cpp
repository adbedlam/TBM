#include "DataBase.h"

void DataBaseLog::log_data(const Candle &data) {
    try {
        pqxx::work txn(conn);  // Используем объект соединения


        txn.exec_prepared(
            "insert_historical",
            data.timestamp,
            data.price,
            data.volume,
            data.symbol
        );

        txn.commit();
    }
    catch (const std::exception &e) {
        std::cerr << "Error logging historical data: " << e.what() << std::endl;

    }
}

void DataBaseLog::log_data(const string& symbols, const double &macd, const double &signal, const bool &supertrend, const double &ATR,
                           const double &Bbands_u, const double &Bbands_l,
                           const double &Bbands_m, const double &price, const uint64_t &time)
{

    try {
        pqxx::work txn(conn);

        txn.exec_prepared(
            "insert_indicators",
            symbols, macd, signal, supertrend, ATR, Bbands_u, Bbands_l, Bbands_m, price, time
        );
        txn.commit();
    }
    catch (const std::exception& e) {
        std::cerr << "Error logging indicators: " << e.what() << std::endl;

    }
}

void DataBaseLog::log_data(uint64_t &timestamp, string &action, string &symbol, double &quant, double &price, double &commision, string& strategy)

{
    try {
        pqxx::work txn(conn);


        txn.exec_prepared(
            "insert_transactions",
            timestamp,
            action,
            symbol,
            quant,
            price,
            commision,
            strategy
        );
        txn.commit();
    }
    catch (const std::exception& e) {
        std::cerr << "Error logging transaction: " << e.what() << std::endl;

    }
}