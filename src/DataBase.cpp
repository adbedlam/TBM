#include "DataBase.h"

void DataBaseLog::log_data(const Candle &data, const double& usdt_balance, const double&  coin_balance) {
    try {
        pqxx::work txn(conn);  // Используем объект соединения


        txn.exec_prepared(
            "insert_historical",
            data.timestamp,
            data.price,
            usdt_balance,
            coin_balance,
            data.symbol
        );

        txn.commit();
    }
    catch (const std::exception &e) {
        std::cerr << "Error logging historical data: " << e.what() << std::endl;

    }
}

void DataBaseLog::log_data(const string& symbols, const double &macd, const double &signal,
                  const double &Bbands_u, const double &Bbands_l,const double &Bbands_m,
                  const double &ts, const double &ks, const double &ssa, const double &ssb,
                  const double &ma20, const double &ma50, const double &obv,
                  const double &rsi, const double &price, const uint64_t &time)
{

    try {
        pqxx::work txn(conn);

        txn.exec_prepared(
            "insert_indicators",
            symbols, macd, signal, Bbands_u, Bbands_l, Bbands_m, ts, ks, ssa, ssb, ma20, ma50, obv, rsi, price, time
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