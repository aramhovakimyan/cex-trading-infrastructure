#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>

#include <nlohmann/json.hpp>

#include "market_data/market_data_parser.hpp"
#include "market_data/order_book.hpp"

#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>

namespace net = boost::asio;
namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace ssl = net::ssl;

using tcp = net::ip::tcp;
using json = nlohmann::json;

class BinanceMarketDataClient {
public:
    BinanceMarketDataClient()
        : ioc_(),
          ssl_context_(ssl::context::tlsv12_client),
          resolver_(ioc_),
          ws_(ioc_, ssl_context_) {

        ssl_context_.set_default_verify_paths();
        ssl_context_.set_verify_mode(ssl::verify_peer);
    }

    
    void run() {
     const std::string host =
        "stream.testnet.binance.vision";

    const std::string port = "9443";

    const std::string target =
        "/ws/btcusdt@bookTicker";

    try {
        auto const endpoints =
            resolver_.resolve(host, port);

        // Establish TCP connection
        beast::get_lowest_layer(ws_).connect(endpoints);

        // Configure TLS hostname verification
        if (!SSL_set_tlsext_host_name(
                ws_.next_layer().native_handle(),
                host.c_str())) {

            throw beast::system_error(
                beast::error_code(
                    static_cast<int>(::ERR_get_error()),
                    net::error::get_ssl_category()
                )
            );
        }

        // Verify server certificate
        ws_.next_layer().set_verify_callback(
            ssl::host_name_verification(host)
        );

        // TLS handshake
        ws_.next_layer().handshake(
            ssl::stream_base::client
        );

        // WebSocket handshake
        ws_.handshake(
            host + ":" + port,
            target
        );

        std::cout
            << "Connected to Binance Testnet\n";

        listen();

    } catch (const std::exception& e) {
        std::cerr
            << "Connection error: "
            << e.what() << '\n';
    }
}

private:
    void listen() {
        beast::flat_buffer buffer;

        while (true) {
            buffer.clear();

            ws_.read(buffer);

            const std::string message =
                beast::buffers_to_string(buffer.data());

            process_message(message);
        }
    }

    void process_message(const std::string& message) {
        auto event =
            trading::MarketDataParser::parse_book_ticker(
                message
            );

        if (!event) {
            std::cerr << "Invalid market data message\n";
            return;
        }

        order_book_.update(*event);

        const auto snapshot =
            order_book_.get_snapshot();

        if (!snapshot) {
            return;
        }

        const double spread = order_book_.spread();
        const double midpoint = order_book_.midpoint();

        std::cout
            << snapshot->symbol
            << " | Bid: " << snapshot->bid_price
            << " | Ask: " << snapshot->ask_price
            << " | Spread: " << spread
            << " | Midpoint: " << midpoint
            << '\n';
    }

    net::io_context ioc_;
    ssl::context ssl_context_;
    tcp::resolver resolver_;
    websocket::stream<beast::ssl_stream<beast::tcp_stream>> ws_;
    trading::OrderBook order_book_;
};

int main() {
    BinanceMarketDataClient client;

    client.run();

    return EXIT_SUCCESS;
}

