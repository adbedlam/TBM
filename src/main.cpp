#include "APIread.h"
#include "BinanceWebsocket.h"
#include "ERBB.h"
#include "BinanceAPI.h"
#include "OrderManager.h"




atomic<bool> running{true};

void signal_handler(int signum) {
    cout << "\nInterrupt signal (" << signum << ") received.\n";
    running = false;
}



int main() {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);


    // Параметры для Ордера Бинанс
    double min_price = 10.0;

    double min_price_btc = 0.001;

    double step_size = 0.001;


    // Параметры стратегии, ПОДУМАТЬ
    auto ema_short = 9 * 24 * 360 * 1000;
    auto ema_long = 21 * 24 * 360 * 1000;
    auto rsi_period = 14 * 24 * 360 * 1000;
    auto bb_period = 16 * 24 * 360 * 1000;

    auto bb_std_dev = 2.0;
    auto overbought = 70.0;
    auto oversold = 30.0;


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
    DataEMA_RSI_BB strategy(ema_short, ema_long, rsi_period, bb_period, bb_std_dev, overbought, oversold);

    strategy.set_trade_callback([&](const string &action, double price) {
        double initial_balance_USDT = acc_manager.get_balance("USDT");
        double initial_balance_BTC = acc_manager.get_balance("BTC");

        double quant;

        if (action == "BUY") {
            initial_balance_USDT = acc_manager.get_balance("USDT");


            quant = std::min(initial_balance_USDT * 0.02, min_price);
            quant = std::floor((quant / price) / step_size) * step_size;
        } else {
            initial_balance_BTC = acc_manager.get_balance("BTC");


            quant = std::min(initial_balance_BTC * 0.02, min_price_btc);

            quant = std::floor(quant / step_size) * step_size;
        }


        const string symbol = "BTCUSDT";
        const double quantity = quant;

        try {
            order_manager.add_order(
                action,
                symbol,
                price,
                quantity
            );
        } catch (const exception &e) {
            cerr << "Error adding order: " << e.what() << endl;
        }
    });



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
                cout << data.dump(2) << "\n\n";
                DataCSV event{
                    data["E"].get<uint64_t>(), // timestamp
                    data["s"].get<string>(), // symbol
                    stod(data["k"]["c"].get<string>()), // price
                    stod(data["k"]["v"].get<string>()) // volume
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
                strategy.get_bollinger_bands(upper_bb, middle_bb, lower_bb);

                cout << "====== Indicators " << std::put_time(&tm_info, "%F %T") << " ======" << endl;
                cout << "EMA Short: " << strategy.get_short_ema() << endl;
                cout << "EMA Long: " << strategy.get_long_ema() << endl;
                cout << "RSI: " << strategy.get_rsi() << endl;
                cout << "Bollinger Bands: " << upper_bb << " | "
                        << middle_bb << " | " << lower_bb << endl;
                cout << "Current Price: " << event.price << "\n\n";



                {
                       lock_guard<std::mutex> lock(data_mutex);
                       data30_s.last_event = event;
                       data30_s.ema_short = strategy.get_short_ema();
                       data30_s.ema_long = strategy.get_long_ema();
                       data30_s.rsi = strategy.get_rsi();
                       data30_s.upper_bb = upper_bb;
                       data30_s.lower_bb = lower_bb;
                       data30_s.middle_bb = middle_bb;
                       data30_s.has_data = true;
                }



                strategy.update(event);

            } catch (const exception &e) {
                cerr << "WS data error: " << e.what() << endl;
            }
        }
    });



    // Подключение WebSocket
    ws->connect("btcusdt@kline_1m");


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

            cout << "====== Status "<< std::put_time(&tm_info, "%F %T") << " ======" << endl;
            cout << "Total profit: " << acc_manager.get_profit(total_balance) << " USDT" << endl;
            cout << "Active orders: " << order_manager.queue_size() << endl;
            cout << "Current balance:" << endl;
            cout << "BTC: " << acc_manager.get_balance("BTC") << " | USDT: "
                    << acc_manager.get_balance("USDT") << "\n\n";

            std::this_thread::sleep_for(2s);
        }
    });


    // Поток для логирования данных (индикаторы, исторические), каждые 30с
    thread db_log_data_thread([&]() {
        while (running) {
           std::this_thread::sleep_for(30s);

           Data30s local_data;
           {
               std::lock_guard<std::mutex> lock(data_mutex);
               if (!data30_s.has_data) continue;
               local_data = data30_s;
           }

           try {

               logger.log_data(local_data.last_event);

               logger.log_data(
                   local_data.ema_short,
                   local_data.ema_long,
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


