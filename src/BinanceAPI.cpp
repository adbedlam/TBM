#include "BinanceAPI.h"

BinanceAPIc::BinanceAPIc(const string &api_key, const string &secret_key) {
    this->api_key = api_key;
    this->secret_key = secret_key;
    base_url = "https://testnet.binance.vision";
}

json BinanceAPIc::create_order(const string &symbol, const string &side, const string &type, double quantity,
                               double price) {
    int64_t timestamp = get_server_time();
    map<string, string> params = {
        {"symbol", symbol},
        {"side", side},
        {"type", type},
        {"quantity", std::to_string(quantity)},
        {"timestamp", to_string(timestamp)}
    };

    if (type == "LIMIT") {
        params["price"] = to_string(price);
        params["timeInForce"] = "GTC";
    }

    return http_request("POST", "/api/v3/order", params);
}


string BinanceAPIc::sign_request(const string &query) {
    unsigned char *digest = HMAC(EVP_sha256(), secret_key.c_str(),
                                 secret_key.length(), (unsigned char *) query.c_str(),
                                 query.length(), nullptr, nullptr);

    stringstream ss;
    for (int i = 0; i < 32; ++i) {
        ss << hex << setw(2) << setfill('0') << (int) digest[i];
    }
    return ss.str();
}


json BinanceAPIc::http_request(const string &method, const string &endpoint, const map<string, string> &params,
                               bool flag_time) {
    string query;

    for (const auto &[key, val]: params) {
        if (!query.empty()) {
            query += "&";
        }
        query += key + "=" + val;
    }

    if (endpoint == "/api/v3/account") {
        query += "timestamp=" + to_string(get_current_timestamp_ms());
    }

    if (!flag_time) {
        string sign = sign_request(query);
        query += "&signature=" + sign;
    }


    net::io_context ioc;
    ssl::context ctx{ssl::context::tlsv13_client};
    beast::ssl_stream<beast::tcp_stream> stream(ioc, ctx);

    net::ip::tcp::resolver resolver(ioc);

    auto const results = resolver.resolve(
        base_url.substr(8),
        "443"
    );

    beast::get_lowest_layer(stream).connect(results);


    string host = base_url.substr(8);
    if (!SSL_set_tlsext_host_name(stream.native_handle(), host.c_str())) {
        throw std::runtime_error("Failed to set SNI");
    }

    stream.handshake(ssl::stream_base::client);


    beast::http::request<beast::http::string_body> req{beast::http::string_to_verb(method), endpoint + "?" + query, 11};

    req.set(beast::http::field::host, base_url.substr(8));
    req.set(beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    req.set("X-MBX-APIKEY", api_key);


    beast::http::write(stream, req);

    beast::flat_buffer buffer;
    beast::http::response<beast::http::string_body> res;

    beast::http::read(stream, buffer, res);


    beast::error_code ec;

    stream.shutdown(ec);


    return json::parse(res.body());
}

int64_t BinanceAPIc::get_server_time() {
    json response = http_request("GET", "/api/v3/time", {}, true);
    return response["serverTime"].get<int64_t>();
}

int64_t BinanceAPIc::get_current_timestamp_ms() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()
    );
    return ms.count();
}
