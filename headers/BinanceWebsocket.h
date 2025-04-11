#ifndef BINANCEWEBSOCKET_H
#define BINANCEWEBSOCKET_H
#include "common.hpp"
#include "strategy.h"


namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace ssl = net::ssl;
using tcp = net::ip::tcp;
using json = nlohmann::json;

using std::cerr;
using std::cout;
using std::endl;

using std::string;
using std::exception;

using std::function;
using std::enable_shared_from_this;


class Websocket : public enable_shared_from_this<Websocket> {
private:
    websocket::stream<beast::ssl_stream<beast::tcp_stream>> ws_;

    tcp::resolver resolver_{ws_.get_executor()};

    beast::flat_buffer buffer_;

    string host_;
    string endpoint_;

    string api_key;
    string secret_key;

    function<void(const json &)> callback_;

    DataSMA data_sma_calc;


    void on_resolve(beast::error_code ec, tcp::resolver::results_type results);

    void on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type);

    void on_ssl_handshake(beast::error_code ec);

    void on_handshake(beast::error_code ec);

    void read_f();

    void on_read(beast::error_code ec, size_t bytes_transfer);

    void fail(beast::error_code ec, char const *ew);

public:
    Websocket(net::io_context &ioc, ssl::context &ctx, const string &api, const string &secret) :
        resolver_(net::make_strand(ioc)), ws_(net::make_strand(ioc), ctx), api_key(api), secret_key(secret) {}

    void connect(const string &symbol);

    void on_message(function<void(const json &)> callback);
};


#endif //BINANCEWEBSOCKET_H
