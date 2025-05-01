#ifndef BINANCEAPI_H
#define BINANCEAPI_H
#include "common.hpp"

namespace net = boost::asio;
namespace beast = boost::beast;
namespace ssl = net::ssl;

using json = nlohmann::json;

using std::string;
using std::map;
using std::to_string;
using std::stringstream;
using std::hex;
using std::setw;
using std::setfill;


class BinanceAPIc {
private:
    string api_key;
    string secret_key;
    string base_url;

    string sign_request(const string &query);

    //Время на компе
    int64_t get_current_timestamp_ms();

public:
    int64_t get_server_time();

    BinanceAPIc(const string &api_key, const string &secret_key);

    json create_order(const string &symbol, const string &side, const string &type, double quantity,
                      double price = 0.0);

    json http_request(const string &method, const string &endpoint, const map<string, string> &params = {},
                      bool flag_time = false);

    json get_historical_klines(
            const std::string &symbol,
            const std::string &interval,
            int limit
    );

};


#endif //BINANCEAPI_H
