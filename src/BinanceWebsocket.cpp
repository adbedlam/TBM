#include "BinanceWebsocket.h"

// Private method
void Websocket::on_resolve(beast::error_code ec, tcp::resolver::results_type results) {
    if (ec) {
        return fail(ec, "resolve");
    }

    beast::get_lowest_layer(ws_).async_connect(results,
                                                  beast::bind_front_handler(
                                                  &Websocket::on_connect, shared_from_this()));
}


void Websocket::on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type) {
    if (ec) {
        return fail(ec, "connect");
    }

    if (!SSL_set_tlsext_host_name(ws_.next_layer().native_handle(), host_.c_str())) {
        beast::error_code ec{static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
        return fail(ec, "SNI setup failed");
    }
    ws_.next_layer().async_handshake(ssl::stream_base::client,
                                     beast::bind_front_handler(
                                         &Websocket::on_ssl_handshake, shared_from_this()));
}


void Websocket::on_ssl_handshake(beast::error_code ec) {
    if (ec) {
        return fail(ec, "ssl handshake");
    }



    ws_.async_handshake(host_, endpoint_, beast::bind_front_handler(
                            &Websocket::on_handshake, shared_from_this()));
}


void Websocket::on_handshake(beast::error_code ec) {
    if (ec) {
        return fail(ec, "on handshake");
    }
    read_f();
}

void Websocket::read_f() {
    ws_.async_read(buffer_, beast::bind_front_handler(&Websocket::on_read, shared_from_this()));
}

void Websocket::on_read(beast::error_code ec, size_t bytes_transfer) {
    if (ec) {
        return fail(ec, "on read");
    }
    if (callback_) {
        try {
            auto data = beast::buffers_to_string(buffer_.data());
            callback_(json::parse(data));
        } catch (const exception &e) {
            cerr << "JSON parse error " << e.what() << endl;
        }
    }
    buffer_.consume(bytes_transfer);
    read_f();
}

void Websocket::fail(beast::error_code ec, char const *ew) {
    char buf[1024];
    ERR_error_string_n(ERR_get_error(), buf, sizeof(buf));
    cerr << ew << ": " << ec.message() << " (" << buf << ")\n";
}


// Public method
void Websocket::connect(const string &stream_type) {
    host_ = "testnet.binance.vision";
    endpoint_ = "/ws/" + stream_type;

    resolver_.async_resolve(host_,
                            "443",
                            beast::bind_front_handler(
                                &Websocket::on_resolve,
                                shared_from_this()
                            ));
}

void Websocket::on_message(function<void(const json &)> callback) {
    callback_ = [this, callback](const json& data) {

        callback(data);


        if (data.contains("e") && data["e"] == "btcusdt@kline_30m") {
            DataCSV event{
                data["E"].get<uint64_t>(),
                data["s"].get<std::string>(),
                std::stod(data["c"].get<std::string>()),
                std::stod(data["q"].get<std::string>())
            };


            data_sma_calc.update(event);



        }
    };
}
