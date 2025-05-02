#include "APIread.h"
#include "BinanceWebsocket.h"
#include "BinanceAPI.h"
#include "OrderManager.h"
#include "INDICATORS/BBIndicator.h"
#include "INDICATORS/EMAIndicator.h"
#include "INDICATORS/RSIIndicator.h"
#include "INDICATORS/MACDIndicator.h"
#include "INDICATORS/Supertrend.h"
#include "AnalysisHandler.h"

using json = nlohmann::json;

atomic<bool> running{true};

void signal_handler(int signum) {
    cout << "\nInterrupt signal (" << signum << ") received.\n";
    running = false;
}


// Структура хранящая индикаторы для каждой пары
struct SymbolData {
    EMAIndicator ema200;
    //RSIIndicator rsi;
    BollingerBandsIndicator bb;
    MACDIndicator macd;
    Supertrend supertrend;
    double last_price;

    SymbolData(int ema_p, int bb_p, int macd_s, int macd_l, int macd_sig, int supertrend_p) :
            ema200(ema_p), bb(bb_p), macd(macd_s, macd_l, macd_sig), supertrend(supertrend_p){}

   /* SymbolData(int ema_p, int rsi_p, int bb_p, int macd_s, int macd_l, int macd_sig, int supertrend_p) :
    ema200(ema_p), rsi(rsi_p), bb(bb_p), macd(macd_s, macd_l, macd_sig), supertrend(supertrend_p){} */
};

// Логирование данных
struct LoggedData {
    string symbol;
    double macd;
    double macd_s;
    //double rsi;
    double ATR;
    double bb_up;
    double bb_low;
    double bb_mid;
    double price;
    bool supertrend;
    uint64_t timestamp;
};


unordered_map<string, LoggedData> last_indicators;
mutex indicators_mutex;


int main() {

    // Считывание API ключей
    const auto api_keys = loadConf("../utils/.env");
    const string API = api_keys.at("API_KEY");
    const string SECRET = api_keys.at("SECRET_KEY");
    const string db_password = api_keys.at("DB_PASSWORD");


    // Путь до новых сертификатов
    auto path_to_cacert = "../utils/cacert.pem";




    // Инициализация БД-логера
    const string db_name = "T_B_database";
    const string db_user = "postgres";
    const string db_host = "localhost";
    const string db_port = "5432";


    const string init_db_conn = "dbname="+db_name + " user=" + db_user + " password=" + db_password +
                                " host="+db_host + " port=" + db_port;

    DataBaseLog logger(init_db_conn);

    // Инициализация API
    BinanceAPIc binance_api(API, SECRET);


    // Инициализация менеджеров
    AccountManager acc_manager(binance_api);
    OrderManager order_manager(binance_api, acc_manager,logger, 1); // 5 ордеров/сек


    acc_manager.start();
    order_manager.start();

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);


    double quantity;
    double step_size;
    double min_notional;

    // Параметры стратегии

    // RSI
    auto rsi_period = 14;

    // BB
    auto bb_period = 20;

    // MACD
    auto macd_fast = 12;
    auto macd_slow = 26;
    auto macd_signal = 9;

    // EMA
    auto ema_long = 200;

    int supertrend_period = 10;




    // Валюты по которым вести торги
    const std::vector<string> symbols = {"XRP", "LTC", "ADA", "TRX", "TON"};


    //Создание Объектов индикаторов, для каждой пары
    std::unordered_map<string, SymbolData> symbol_data;
    std::unordered_map<string, AnalysisHandler> indicator_by_symbol;


    for (auto& symbol : symbols) {
        auto data_for_quant = binance_api.http_request("GET", "/api/v3/exchangeInfo", {{"symbol",symbol+"USDT"}}, true);


        for (const auto& mini_data : data_for_quant["symbols"][0]["filters"])
            if (mini_data["filterType"] == "LOT_SIZE") {
                quantity = std::stod(mini_data["minQty"].get<string>());
                step_size = std::stod(mini_data["stepSize"].get<string>());

            }
            else if(mini_data["filterType"] == "NOTIONAL"){
                min_notional = std::stod(mini_data["minNotional"].get<string>());
            }


        symbol_data.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(symbol),
            std::forward_as_tuple(
            ema_long,
            //rsi_period,
            bb_period,
            macd_fast,
            macd_slow,
            macd_signal,
            supertrend_period
            )
        );
        indicator_by_symbol.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(symbol),
            std::forward_as_tuple(quantity, step_size, min_notional)

        );
        cout << "Min quant for: " << symbol << " is " << indicator_by_symbol.at(symbol).get_min_quant() << "\n";
    }


    try {
        for (const auto& coin : symbols) {
            cout << "Loading historical data from Binance... for "<< coin << endl;


            json klines =  binance_api.get_historical_klines(coin, "15m", 200);

            auto& st = symbol_data.at(coin);
            auto& ind = indicator_by_symbol.at(coin);

            // Заполняем стратегию данными из JSON
            for (const auto& candle : klines) {
                Candle data{
                    candle["timestamp"].get<uint64_t>(),
                    candle["symbol"].get<string>(),
                    candle["price"].get<double>(),
                    candle["high"].get<double>(),
                    candle["low"].get<double>(),
                    candle["volume"].get<double>()
            };
                logger.log_data(data, acc_manager.get_balance("USDT"), acc_manager.get_balance(coin));

                st.ema200.update(data);
                //st.rsi.update(data);
                st.bb.update(data);
                st.macd.update(data);
                st.supertrend.update(data);

                ind.set_params(st.supertrend.get_trend(), st.supertrend.get_value(), st.bb.get_bands().bb_up,
                                  st.bb.get_bands().bb_low, st.bb.get_bands().bb_mid,
                                  st.macd.get_macd().macd, st.macd.get_macd().signal,
                                  st.ema200.get_value(), data.price);
            }

            cout << "Successfully loaded " << klines.size() << " historical candles from Binance." << "\n\n";

            // Получаем последнюю цену для start_price
            if (!klines.empty()) {
                st.last_price = klines.back()["price"].get<double>();
            }
        }
    } catch (const exception &e) {
        cerr << "Error loading historical data: " << e.what() << endl;
        return 1;
    }



    // Инициализация WebSocket
    net::io_context ioc;
    ssl::context ctx{ssl::context::tls_client};
    ctx.set_default_verify_paths();
    ctx.set_verify_mode(ssl::verify_peer);
    ctx.load_verify_file(path_to_cacert);
    SSL_CTX_set_cipher_list(ctx.native_handle(),
                            "ECDHE-ECDSA-AES256-GCM-SHA384:"
                            "ECDHE-RSA-AES256-GCM-SHA384:"
                            "ECDHE-ECDSA-CHACHA20-POLY1305:"
                            "ECDHE-RSA-CHACHA20-POLY1305:"
                            "ECDHE-ECDSA-AES128-GCM-SHA256:"
                            "ECDHE-RSA-AES128-GCM-SHA256");

    auto ws = std::make_shared<Websocket>(ioc, ctx, API, SECRET);

    ws->on_message([&](const json &data) {
        if (!running) return;

        if (data.contains("data") && data["data"].contains("e") && data["data"]["e"] == "kline" && data["data"]["k"]["x"] == true) {
            try {

                string full = data["data"]["s"].get<string>();
                string sym  = full.substr(0, full.size()-4);

                auto it = symbol_data.find(sym);
                auto& ind = indicator_by_symbol.at(sym);

                Candle event{
                        data["data"]["E"].get<uint64_t>(),              // timestamp
                        data["data"]["s"].get<string>(),                // symbol
                        stod(data["data"]["k"]["c"].get<string>()),  // close price
                        stod(data["data"]["k"]["h"].get<string>()),  // high
                        stod(data["data"]["k"]["l"].get<string>()),  // low
                        stod(data["data"]["k"]["v"].get<string>())   // volume
                };


                auto now = system_clock::now();
                time_t tt = system_clock::to_time_t(now);

                std::tm tm_info{};
                if (localtime_s(&tm_info, &tt) != 0) {
                   cerr << "Error convert time, Status" << endl;
                }



                logger.log_data(event, acc_manager.get_balance("USDT"), acc_manager.get_balance(sym));

                auto &st = it->second;
                st.ema200.update(event);
                //st.rsi.update(event);
                st.bb.update(event);
                st.macd.update(event);
                st.supertrend.update(event);

                st.last_price = event.price;

                ind.set_params(st.supertrend.get_trend(),st.supertrend.get_value(), st.bb.get_bands().bb_up,
                                 st.bb.get_bands().bb_low, st.bb.get_bands().bb_mid,
                                 st.macd.get_macd().macd, st.macd.get_macd().signal,
                                 st.ema200.get_value(), event.price);

                auto [has_signal, signal_type] = ind.check_signal();

                if (has_signal) {

                    double risk_percent = 3.0;

                    double usdt_balance = acc_manager.get_balance("USDT");

                    double price = symbol_data.at(sym).last_price;

                    string action;

                    if (signal_type.find("BUY") != string::npos) {
                        action = "BUY";
                        quantity = (usdt_balance * risk_percent / 100) / price;

                    }
                    else if (signal_type.find("SELL") != string::npos) {
                        action = "SELL";
                        quantity = indicator_by_symbol.at(sym).get_entry_quantity();
                    }

                    step_size = indicator_by_symbol.at(sym).get_step_size();

                    auto min_q = indicator_by_symbol.at(sym).get_min_notional() / event.price;

                    quantity = std::max({quantity, min_q, indicator_by_symbol.at(sym).get_min_quant()});


                    if (action == "BUY") {
                        indicator_by_symbol.at(sym).open_position(event.price, quantity);
                    }
                    else if (action == "SELL") {
                        indicator_by_symbol.at(sym).close_position(event.price);
                    }


                    quantity = ceil(quantity/step_size) * step_size;

                    cout <<"Quantity: "<< quantity << " for symbol " << sym << " | Price: " << event.price  << "\n\n";

                    order_manager.add_order(action, sym+"USDT", event.price, quantity, signal_type.substr(0,10));
                    cout << "Current Action: " << action << "\n\n";


                }
                //
                // cout << "====== Indicators for "<< sym << " "<< std::put_time(&tm_info, "%F %T") << " ======" << "\n\n";
                // //cout << "RSI: " << st.rsi.get_value() << "\n";
                // cout << "ATR: " << st.supertrend.get_value() << "\n";
                // cout << "Supertrend: " << st.supertrend.get_trend() << "\n";
                // cout << "Bollinger Bands: " << st.bb.get_bands().bb_up << " | "
                //               << st.bb.get_bands().bb_mid << " | " << st.bb.get_bands().bb_low << "\n";
                // cout << "MACD: " << st.macd.get_macd().macd << "\n";
                // cout << "Signal : " << st.macd.get_macd().signal << "\n";
                // cout << "Current Price: " << event.price << "\n\n";
                // cout << "PROFIT for " << sym << ": \n";
                // cout << sym <<": " << acc_manager.get_balance(sym) << " | USDT: " << acc_manager.get_balance("USDT") << "\n";
                // // // cout <<  indicator_by_symbol.at(sym).get_total_profit() << "  USDT "<< indicator_by_symbol.at(sym).get_total_profit_percent() << "% \n\n";



                {
                        lock_guard<std::mutex> lock(indicators_mutex);

                        LoggedData time_log_data;

                        time_log_data.symbol = sym;
                        //time_log_data.rsi = st.rsi.get_value();
                        time_log_data.bb_up = st.bb.get_bands().bb_up;
                        time_log_data.bb_mid = st.bb.get_bands().bb_mid;
                        time_log_data.bb_low = st.bb.get_bands().bb_low;
                        time_log_data.macd = st.macd.get_macd().macd;
                        time_log_data.macd_s = st.macd.get_macd().signal;
                        time_log_data.supertrend = st.supertrend.get_trend();
                        time_log_data.ATR = st.supertrend.get_value();
                        time_log_data.price = event.price;
                        time_log_data.timestamp = event.timestamp;

                        last_indicators[sym] = time_log_data;
                }





            } catch (const exception &e) {
                cerr << "WS data error: " << e.what() << endl;
            }
        }
    });


    // Подключение WebSocket, Параметры свечи либо 24hours@Ticker
    // Создание всех EndPoint
    string streams;

    for (int i = 0; i < symbols.size(); ++i) {
        if (i) {
            streams+='/';
        }
        string s = symbols[i];
        for (auto& c : s) {
            c = tolower(c);
        }
        streams += s + "usdt@kline_15m"; // Параметры свечи
    }


    ws->connect(streams);


    // Поток Вебсокета
    thread ws_thread([&ioc]() {
        while (running) {
            std::this_thread::sleep_for(10s);
            try {
                ioc.run();
            } catch (const exception &e) {
                cerr << "WS error: " << e.what() << endl;
                std::this_thread::sleep_for(1s);
                ioc.restart();
            }
        }
    });



    // Поток мониторинга и создания ордеров
    // thread monitor([&]() {
    //     while (running) {
    //         auto now = system_clock::now();
    //         time_t tt = system_clock::to_time_t(now);
    //
    //         std::tm tm_info{};
    //         if (localtime_s(&tm_info, &tt) != 0) {
    //            cerr << "Error convert time, Status" << endl;
    //         }
    //
    //
    //         //  cout << "====== Status "<< std::put_time(&tm_info, "%F %T") << " ======" << "\n\n";
    //         //  cout << "Active orders: " << order_manager.queue_size() << "\n";
    //         //  cout << "Current balance:" << "\n";
    //         //  for (const auto sym : symbols){
    //         //  cout <<sym << ": " << acc_manager.get_balance(sym) << " | USDT: "
    //         //          << acc_manager.get_balance("USDT") << "\n\n";
    //         // }
    //
    //         std::this_thread::sleep_for(300s);
    //     }
    // });


    // Поток для логирования данных (индикаторы, исторические), каждые 30с
    thread db_log_data_thread([&]() {
        while (running) {
           std::this_thread::sleep_for(30s);



           try {
               unordered_map<string, LoggedData> snapshot;
               {
                lock_guard<mutex> lock(indicators_mutex);
                snapshot = last_indicators;
               }
               for (const auto& [symbol, data] : snapshot) {
                   logger.log_data(
                       symbol,
                       data.macd,
                       data.macd_s,
                       data.supertrend,
                       data.ATR,
                       data.bb_up,
                       data.bb_low,
                       data.bb_mid,
                       data.price,
                       data.timestamp
                   );
           }

               cout << "\n\n Data was written into DB \n\n";
           } catch (const exception& e) {
               cerr << "DB write error: " << e.what() << endl;
           }
        }


    });

    while (running) {
        std::this_thread::sleep_for(1s);
    }

    // Остановка компонентов
    order_manager.stop();
    acc_manager.stop();
    ws_thread.join();
    // monitor.join();
    db_log_data_thread.join();

    return 0;
}


