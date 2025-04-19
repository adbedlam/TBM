//
// Created by nikit on 16.04.2025.
//

#ifndef DATABASE_H
#define DATABASE_H
#include "common.hpp"
#include "strategy.h"

struct Data30s {
    DataCSV last_event;
    double macd = 0;
    double macd_signal = 0;
    double rsi = 0;
    double upper_bb = 0;
    double lower_bb = 0;
    double middle_bb = 0;
    bool has_data = false;
};

class DataBaseLog {
private:
    pqxx::connection conn;

    void create_tables_if_missing(pqxx::connection& c) {
        pqxx::work txn(c);

        //txn.exec("DO $$BEGIN IF NOT EXISTS (SELECT 1 FROM pg_database WHERE datname = 'TBM') THEN CREATE DATABASE TBM;END IF;END $$");

    
        txn.exec(
            "CREATE TABLE IF NOT EXISTS indicators ("
            "id SERIAL PRIMARY KEY,"
            "macd DOUBLE PRECISION NOT NULL,"
            "macd_signal DOUBLE PRECISION NOT NULL,"
            "rsi DOUBLE PRECISION NOT NULL,"
            "upper_band DOUBLE PRECISION NOT NULL,"
            "lower_band DOUBLE PRECISION NOT NULL,"
            "middle_band DOUBLE PRECISION NOT NULL,"
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
            "commission DOUBLE PRECISION NOT NULL)"
        );

        txn.exec(
           "CREATE TABLE IF NOT EXISTS historical ("
           "id SERIAL PRIMARY KEY,"
           "timestamp BIGINT NOT NULL,"
           "price DOUBLE PRECISION NOT NULL,"
           "volume DOUBLE PRECISION NOT NULL,"
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
                "INSERT INTO historical (timestamp, price, volume, symbol) "
                "VALUES ($1, $2, $3, $4)"
            );

            conn.prepare(
                "insert_indicators",
                "INSERT INTO indicators (macd, macd_signal, rsi, upper_band, lower_band, middle_band, price, timestamp) "
                "VALUES ($1, $2, $3, $4, $5, $6, $7, $8)"
            );

            conn.prepare(
                "insert_transactions",
                "INSERT INTO transactions (timestamp, action, symbol, quantity, price, commission) "
                "VALUES ($1, $2, $3, $4, $5, $6)"
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

    void log_data(const DataCSV &data);

    void log_data(const double &sema, const double &lema, const double &rsi,
                  const double &Bbands_u, const double &Bbands_l,
                  const double &Bbands_m, const double &price, uint64_t &time);

    void log_data(uint64_t &timestamp, string &action, string &symbol, double &quant, double &price, double &commision);
};

#endif //DATABASE_H
