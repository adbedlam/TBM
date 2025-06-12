//
// Created by nikit on 16.04.2025.
//

#ifndef DATABASE_H
#define DATABASE_H
#include "common.hpp"

#include "INDICATORS/AbstractStrategy.h"



class DataBaseLog {
private:
    pqxx::connection conn;

    void create_tables_if_missing(pqxx::connection& c) {
        pqxx::work txn(c);

        txn.exec("DO $$BEGIN IF NOT EXISTS (SELECT 1 FROM pg_database WHERE datname = 't_b_database') THEN CREATE DATABASE T_B_database;END IF;END $$");

    
        txn.exec(
            "CREATE TABLE IF NOT EXISTS indicators ("
            "id SERIAL PRIMARY KEY,"
            "symbols VARCHAR(10) NOT NULL,"

            "macd DOUBLE PRECISION NOT NULL,"
            "signal DOUBLE PRECISION NOT NULL,"

            "upper_band DOUBLE PRECISION NOT NULL,"
            "lower_band DOUBLE PRECISION NOT NULL,"
            "middle_band DOUBLE PRECISION NOT NULL,"

            "ts DOUBLE PRECISION NOT NULL,"
            "ks DOUBLE PRECISION NOT NULL,"
            "ssa DOUBLE PRECISION NOT NULL,"
            "ssb DOUBLE PRECISION NOT NULL,"

            "ma20 DOUBLE PRECISION NOT NULL,"
            "ma50 DOUBLE PRECISION NOT NULL,"

            "obv DOUBLE PRECISION NOT NULL,"

            "rsi DOUBLE PRECISION NOT NULL,"

            "price DOUBLE PRECISION NOT NULL,"

            "timestamp BIGINT NOT NULL)"
        );

        txn.exec(
            "CREATE TABLE IF NOT EXISTS transactions ("
            "id SERIAL PRIMARY KEY,"
            "timestamp BIGINT NOT NULL,"
            "action VARCHAR(32) NOT NULL,"
            "symbol VARCHAR(16) NOT NULL,"
            "quantity DOUBLE PRECISION NOT NULL,"
            "price DOUBLE PRECISION NOT NULL,"
            "commission DOUBLE PRECISION NOT NULL,"
            "indicator VARCHAR(16) NOT NULL)"
        );

        txn.exec(
           "CREATE TABLE IF NOT EXISTS historical ("
           "id SERIAL PRIMARY KEY,"
           "timestamp BIGINT NOT NULL,"
           "price DOUBLE PRECISION NOT NULL,"
           "usdt_balance DOUBLE PRECISION NOT NULL,"
           "coin_balance DOUBLE PRECISION NOT NULL,"
           "symbol VARCHAR(16) NOT NULL)"
        );


        txn.commit();
    }

    bool statements_prepared = false;

    void prepare_statements() {
        if (statements_prepared) return;

        try {

            conn.prepare(
                "insert_historical",
                "INSERT INTO historical (timestamp, price, usdt_balance, coin_balance, symbol) "
                "VALUES ($1, $2, $3, $4, $5)"
            );

            conn.prepare(
                "insert_indicators",
                "INSERT INTO indicators (symbols, macd, signal, upper_band, lower_band, middle_band, ts, ks, ssa, ssb, ma20, ma50, obv, rsi, price, timestamp) "
                "VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13, $14, $15, $16)"
            );

            conn.prepare(
                "insert_transactions",
                "INSERT INTO transactions (timestamp, action, symbol, quantity, price, commission, indicator) "
                "VALUES ($1, $2, $3, $4, $5, $6, $7)"
            );

            statements_prepared = true;
        }

        catch (const std::exception& e) {
            std::cerr << "Prepare statements error: " << e.what() << std::endl;
            throw;
        }
    }

public:
    DataBaseLog(const string &conn_string) : conn(conn_string){
        try {

            if (!conn.is_open()) {
                throw std::runtime_error("Conection failed");
            }
            create_tables_if_missing(conn);
            prepare_statements();
            std::cout << "Database initialized successfully" << std::endl;

        }
        catch (const std::exception& e) {
            std::cerr << "DATABASE ERROR: " << e.what() << std::endl;
            throw;
        }
    }

    void log_data(const Candle &data, const double& usdt_balance, const double&  coin_balance);

    void log_data(const string& symbols, const double &macd, const double &signal,
                  const double &Bbands_u, const double &Bbands_l,const double &Bbands_m,
                  const double &ts, const double &ks, const double &ssa, const double &ssb,
                  const double &ma20, const double &ma50, const double &obv,
                  const double &rsi, const double &price, const uint64_t &time);

    void log_data(uint64_t &timestamp, string &action, string &symbol, double &quant, double &price, double &commision, string& strategy);
};

#endif //DATABASE_H
