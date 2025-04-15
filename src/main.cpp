#include "APIread.h"
#include "BinanceWebsocket.h"
#include "DataLog.h"
#include "strategy.h"
#include "BinanceAPI.h"
#include "OrderManager.h"

atomic<bool> running{true};

void signal_handler(int signum) {
    cout << "\nInterrupt signal (" << signum << ") received.\n";
    running = false;
}

const double min_price{10.0};
const double min_price_btc{0.001};

const double step_size{0.001};

int main() {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);



    // Параметры для принятия решений
    auto short_window = 5;
    auto long_windows = 20;
    double threashold = 0.001;

    // Считывание Апи ключей
    const auto api_keys = loadConf("../utils/.env");

    const string API = api_keys.at("API_KEY");
    const string SECRET = api_keys.at("SECRET_KEY");

    // Путь к новому сертификату
    auto path_to_cacert = "../utils/cacert.pem";


    // Инициалиазция робота
    BinanceAPIc binance_api(api_keys.at("API_KEY"), api_keys.at("SECRET_KEY"));


    // Иницаилизация управленцев
    AccountManager acc_manager(binance_api);
    OrderManager order_manager(binance_api, acc_manager, 1); // 5 ордеров/сек

    acc_manager.start();
    order_manager.start();



    // Иниациализация сборщика
    DataSMA data_sma(short_window, long_windows, threashold); // Поменять на нормальные параметры

    data_sma.set_trade_callback([&](const string &action, double price) {

        double quant;
        double initial_balance_BTC;
        double initial_balance_USDT;


        if (action == "BUY") {
            initial_balance_USDT = acc_manager.get_balance("USDT");


            quant = max(initial_balance_USDT*0.15, min_price);
            quant =  std::floor((quant / price) / step_size) * step_size;
        }
        else {
           initial_balance_BTC = acc_manager.get_balance("BTC");


            quant = std::max(initial_balance_BTC*0.02, min_price_btc);

            quant = std::floor(quant / step_size) * step_size;
        }
        const string symbol = "BTCUSDT";

        // Торги по 15% процентов от всего баланса
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


    // Параметры для Websocket'a

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


    // Инициализация Websocket'a
    auto ws = std::make_shared<Websocket>(ioc, ctx, API, SECRET);


    ws->on_message([&](const json &data) {
        if (!running) return;

        if (data.contains("e") && data["e"] == "24hrTicker") {
            try {
                DataCSV event{
                    data["E"].get<uint64_t>(), // timestamp
                    data["s"].get<string>(), // symbol
                    stod(data["c"].get<string>()), // price
                    stod(data["q"].get<string>()) // volume
                };

                // Логирование данных
                log_data(event);

                // Обновление стратегии
                data_sma.update(event);
            } catch (const exception &e) {
                cerr << "WS data error: " << e.what() << endl;
            }
        }
    });


    // Подключение Websoket'a по конкретной паре добавить пары
    ws->connect("btcusdt@ticker");

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

    thread monitor([&]() {
        while (running) {
            cout << "=== Status ===\n"
                    << "Total profit: " << acc_manager.get_profit() << " USDT\n"
                    << "Active orders: " << order_manager.queue_size() << "\n\n"
                    << "Current balance: \n" << "BTC: "
                    << acc_manager.get_balance("BTC") << " | USDT: "
                    << acc_manager.get_balance("USDT") << "\n\n";

            std::this_thread::sleep_for(2s);
        }
    });

    while (running) {
        std::this_thread::sleep_for(1s);
    }


    order_manager.stop();
    acc_manager.stop();
    ws_thread.join();
    monitor.join();


    return 0;
}


/*
 * E - текущее время
 * e - Свод. статистика за 24 часа
 * s - пара
 * p - абсолютное изменение цены за 24 часа
 * P - изменение в %
 * w - взвешенная средняя цена
 * x - закрытая цена предыдущего периода
 * c - текущая цена
 * b - лучшая цена покупки
 * B - Объём лучшей покупки
 * а - лучшая цена продажи
 * A - Объем лучшей продажи
 * о - цена открытия
 * h - Максимальная цена за 24 часа.
 * l - Минимальная цена за 24 часа.
 * v - Объём торгов в базовой валюте за 24 часа.
 * q - Объём торгов в котируемой валюте за 24 часа.
 *
 */
