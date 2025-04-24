#include "APIread.h"
#include "BinanceWebsocket.h"
#include "ERBB.h"
#include "BinanceAPI.h"
#include "OrderManager.h"
#include "MACDST.h"

using json = nlohmann::json;

atomic<bool> running{true};

void signal_handler(int signum) {
    cout << "\nInterrupt signal (" << signum << ") received.\n";
    running = false;
}


int main() {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);


    bool wait = true;

    // Параметры для Ордера Бинанс
    double min_price = 10.0;

    double min_price_btc = 0.001;

    double step_size = 0.001;


    // Параметры стратегии, ПОДУМАТЬ
    auto rsi_period = 14 * 48; // * 24 * 360 * 1000;
    auto bb_period = 20 * 48; //* 24 * 360 * 1000;

    auto bb_std_dev = 2.0;
    auto overbought = 70.0;
    auto oversold = 30.0;

    auto macd_fast = 12 * 48;
    auto macd_slow = 26 * 48;
    auto macd_signal = 9 * 48;
    auto ema_long = 200 * 48;

    // Считывание API ключей
    const auto api_keys = loadConf("../utils/.env");
    const string API = api_keys.at("API_KEY");
    const string SECRET = api_keys.at("SECRET_KEY");
    const string db_password = api_keys.at("DB_PASSWORD");


    // Путь до новых сертификатов
    auto path_to_cacert = "../utils/cacert.pem";


    // Инициализация БД-логера
    const string db_name = "TBM";
    const string db_user = "postgres";
    const string db_host = "localhost";
    const string db_port = "5432";

    const string init_db_conn = "dbname="+db_name + " user=" + db_user + " password=" + db_password +
                                " host="+db_host + " port=" + db_port;

    DataBaseLog logger(init_db_conn);

    Data30s data30_s;
    mutex data_mutex;


    // Инициализация API
    BinanceAPIc binance_api(API, SECRET);


    // Инициализация менеджеров
    AccountManager acc_manager(binance_api);
    OrderManager order_manager(binance_api, acc_manager,logger, 1); // 5 ордеров/сек


    acc_manager.start();
    order_manager.start();


    double start_price{0};
    double base_val_btc = acc_manager.get_balance("BTC");
    const double base_val_usdt = acc_manager.get_balance("USDT");



    // Инициализация стратегии EMA+RSI+Bollinger Bands
    DataEMA_RSI_BB strategy_rsi_bb(rsi_period, bb_period,
                            bb_std_dev, overbought,
                            oversold);


    DataMACD_ST strategy_macd(macd_fast, macd_slow, macd_signal,
                              ema_long, 3.0, 10);


    strategy_rsi_bb.set_trade_callback([&](const string &action, double price) {
        double quant;
        double balance;
        double min_order;
        double percent = 0.1;

        // Проверка баланса перед сделкой
        if (action == "LONG_BUY") {
            balance = acc_manager.get_balance("USDT");
            min_order = min_price;
            if (balance < min_order) {
                cout << "Insufficient USDT balance for " << action << endl;
                return;
            }
            quant = std::min(balance * percent, min_order);
        } else {
            balance = acc_manager.get_balance("BTC");
            min_order = min_price_btc;
            if (balance < min_order) {
                cout << "Insufficient BTC balance for " << action << endl;
                return;
            }
            quant = balance * percent;
        }

        quant = std::floor((action.find("BUY") != string::npos ?
                            (quant / price) / step_size : quant) / step_size) * step_size;

        if (quant <= 0) {
            cout << "Calculated quantity is zero for " << action << endl;
            return;
        }

        try {
            order_manager.add_order(
                    action.substr(action.find('_') + 1),
                    "BTCUSDT",
                    price,
                    quant
            );
        } catch (const exception &e) {
            cerr << "Error adding order: " << e.what() << endl;
        }
    });


    strategy_macd.set_trade_callback([&](const string &action, double price) {
        double quant;
        double balance;
        double min_order;
        double percent = 0.05; // 5% для стратегии MACD

        if (action == "MACD_BUY") {
            balance = acc_manager.get_balance("USDT");
            min_order = min_price;
            if (balance < min_order) {
                cout << "Insufficient USDT balance for " << action << endl;
                return;
            }
            quant = std::min(balance * percent, min_order);
        } else {
            balance = acc_manager.get_balance("BTC");
            min_order = min_price_btc;
            if (balance < min_order) {
                cout << "Insufficient BTC balance for " << action << endl;
                return;
            }
            quant = balance * percent;
        }

        double normalized_quant;
        if (action.find("BUY") != string::npos) {

            normalized_quant = (quant / price) / step_size;
        } else {

            normalized_quant = quant / step_size;
        }

        quant = std::floor(normalized_quant) * step_size;

        if (quant <= 0) {
            cout << "Calculated quantity is zero for " << action << endl;
            return;
        }

        try {
            order_manager.add_order(
                    action.substr(action.find('_') + 1),
                    "BTCUSDT",
                    price,
                    quant
            );
        } catch (const exception &e) {
            cerr << "Error adding order: " << e.what() << endl;
        }
    });

    try {
        cout << "Loading historical data from JSON file..." << endl;

        // Загрузка данных из JSON файла
        json historical_data = loadJsonData("../utils/binance_30m_candles.json");

        if (!historical_data.is_array()) {
            throw std::runtime_error("Invalid JSON data format");
        }

        // Заполняем стратегию данными из JSON
        for (const auto& candle : historical_data) {
            DataCSV data{
                    candle["timestamp"].get<uint64_t>(),
                    candle["symbol"].get<string>(),
                    candle["price"].get<double>(),
                    candle["high"].get<double>(),   // добавлено
                    candle["low"].get<double>(),    // добавлено
                    candle["volume"].get<double>()
            };

            strategy_rsi_bb.update(data);
            strategy_macd.update(data);
        }

        cout << "Successfully loaded " << historical_data.size() << " historical candles from JSON file." << endl;


        // Получаем последнюю цену для start_price
        if (!historical_data.empty()) {
            start_price = historical_data.back()["price"].get<double>();
        }
    } catch (const exception &e) {
        cerr << "Error loading historical data: " << e.what() << endl;
        return 1;
    }



    // Инициализация WebSocket
    net::io_context ioc;
    ssl::context ctx{ssl::context::tlsv13_client};
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

        if (data.contains("e") && data["e"] == "kline") {
            try {

                DataCSV event{
                        data["E"].get<uint64_t>(),    // timestamp
                        data["s"].get<string>(),      // symbol
                        stod(data["k"]["c"].get<string>()),  // close price
                        stod(data["k"]["h"].get<string>()),  // high
                        stod(data["k"]["l"].get<string>()),  // low
                        stod(data["k"]["v"].get<string>())   // volume
                };


                if (start_price == 0) {
                    start_price = event.price;
                }

                auto now = system_clock::now();
                time_t tt = system_clock::to_time_t(now);

                std::tm tm_info{};
                if (localtime_s(&tm_info, &tt) != 0) {
                   cerr << "Error convert time, Status" << endl;
                }


                // Вывод текущих значений индикаторов
                double upper_bb, middle_bb, lower_bb;
                strategy_rsi_bb.get_bollinger_bands(upper_bb, middle_bb, lower_bb);
                double macd_v, signal_v, histogram_v;
                strategy_macd.get_macd(macd_v,signal_v,histogram_v);


                cout << "====== Indicators " << std::put_time(&tm_info, "%F %T") << " ======" << "\n\n";
                // cout << "EMA Short: " << strategy.get_short_ema() << endl;
                // cout << "EMA Long: " << strategy.get_long_ema() << endl;
                cout << "RSI: " << strategy_rsi_bb.get_rsi() << endl;
                cout << "Bollinger Bands: " << upper_bb << " | "
                        << middle_bb << " | " << lower_bb << endl;
                cout << "Current Price: " << event.price << "\n\n";



                {
                       lock_guard<std::mutex> lock(data_mutex);
                       data30_s.last_event = event;
                       data30_s.macd = macd_v;
                       data30_s.macd_signal = signal_v;
                       data30_s.rsi = strategy_rsi_bb.get_rsi();
                       data30_s.upper_bb = upper_bb;
                       data30_s.lower_bb = lower_bb;
                       data30_s.middle_bb = middle_bb;
                       data30_s.has_data = true;
                }



                strategy_rsi_bb.update(event);
                strategy_macd.update(event);


            } catch (const exception &e) {
                cerr << "WS data error: " << e.what() << endl;
            }
        }
    });



    // Подключение WebSocket, Параметры свечи либо 24hours@Ticker
    ws->connect("btcusdt@kline_30m");


    // Поток Вебсокета
    thread ws_thread([&ioc]() {
        while (running) {
            try {
                ioc.run();
            } catch (const exception &e) {
                cerr << "WS error: " << e.what() << endl;
                std::this_thread::sleep_for(1s);
                ioc.restart();
            }
        }
    });
    double total_balance = (start_price * base_val_btc) + base_val_usdt;


    // Поток мониторинга и создания ордеров
    thread monitor([&]() {
        while (running) {
            auto now = system_clock::now();
            time_t tt = system_clock::to_time_t(now);

            std::tm tm_info{};
            if (localtime_s(&tm_info, &tt) != 0) {
               cerr << "Error convert time, Status" << endl;
            }

            cout << "====== Status "<< std::put_time(&tm_info, "%F %T") << " ======" << "\n\n";
            cout << "Total profit: " << acc_manager.get_profit(total_balance) << " USDT" << "\n";
            cout << "Active orders: " << order_manager.queue_size() << "\n";
            cout << "Current balance:" << "\n";
            cout << "BTC: " << acc_manager.get_balance("BTC") << " | USDT: "
                    << acc_manager.get_balance("USDT") << "\n\n";

            std::this_thread::sleep_for(2s);
        }
    });


    // Поток для логирования данных (индикаторы, исторические), каждые 30с
    thread db_log_data_thread([&]() {
        while (running) {
           std::this_thread::sleep_for(305s);

           Data30s local_data;
           {
               std::lock_guard<std::mutex> lock(data_mutex);
               if (!data30_s.has_data) continue;
               local_data = data30_s;
           }

           try {

               logger.log_data(local_data.last_event);

               logger.log_data(
                   local_data.macd,
                   local_data.macd_signal,
                   local_data.rsi,
                   local_data.upper_bb,
                   local_data.lower_bb,
                   local_data.middle_bb,
                   local_data.last_event.price,
                   local_data.last_event.timestamp
               );
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
    monitor.join();
    db_log_data_thread.join();

    return 0;
}


