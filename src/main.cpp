#include "APIread.h"
#include "BinanceWebsocket.h"
#include "DataLog.h"
#include "e_r_bb.h"
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

    // Параметры стратегии
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
    auto path_to_cacert = "../utils/cacert.pem";

    // Инициализация API
    BinanceAPIc binance_api(API, SECRET);

    // Инициализация менеджеров
    AccountManager acc_manager(binance_api);
    OrderManager order_manager(binance_api, acc_manager, 5); // 5 ордеров/сек

    acc_manager.start();
    order_manager.start();

    // Инициализация стратегии EMA+RSI+Bollinger Bands
    DataEMA_RSI_BB strategy(ema_short, ema_long, rsi_period,
                            bb_period, bb_std_dev,
                            overbought, oversold);

    strategy.set_trade_callback([&](const string &action, double price) {
        const string symbol = "BTCUSDT";
        const double quantity = 0.0001; // Фиксированный объем

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

                // Вывод текущих значений индикаторов
                double upper_bb, middle_bb, lower_bb;
                strategy.get_bollinger_bands(upper_bb, middle_bb, lower_bb);

                cout << "=== Indicators ===" << endl;
                cout << "EMA Short: " << strategy.get_short_ema() << endl;
                cout << "EMA Long: " << strategy.get_long_ema() << endl;
                cout << "RSI: " << strategy.get_rsi() << endl;
                cout << "Bollinger Bands: " << upper_bb << " | "
                     << middle_bb << " | " << lower_bb << endl;
                cout << "Current Price: " << event.price << "\n\n";

                // Обновление стратегии
                strategy.update(event);
            } catch (const exception &e) {
                cerr << "WS data error: " << e.what() << endl;
            }
        }
    });

    // Подключение WebSocket
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
            cout << "=== Status ===" << endl;
            cout << "Total profit: " << acc_manager.get_profit() << " USDT" << endl;
            cout << "Active orders: " << order_manager.queue_size() << endl;
            cout << "Current balance:" << endl;
            cout << "BTC: " << acc_manager.get_balance("BTC") << " | USDT: "
                 << acc_manager.get_balance("USDT") << "\n\n";
            std::this_thread::sleep_for(2s);
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
