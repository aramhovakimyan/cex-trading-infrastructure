#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>

#include <curl/curl.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

#include <atomic>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

#include "market_data/bounded_queue.hpp"
#include "market_data/depth_parser.hpp"
#include "market_data/depth_synchronizer.hpp"
#include "market_data/order_book.hpp"

namespace net = boost::asio;
namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace ssl = net::ssl;

using tcp = net::ip::tcp;

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

    void connect() {
        const std::string host =
            "stream.testnet.binance.vision";

        const std::string port = "9443";

        const std::string target =
            "/ws/btcusdt@depth@100ms";

        auto endpoints = resolver_.resolve(host, port);

        beast::get_lowest_layer(ws_).connect(endpoints);

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

        ws_.next_layer().set_verify_callback(
            ssl::host_name_verification(host)
        );

        ws_.next_layer().handshake(
            ssl::stream_base::client
        );

        ws_.handshake(
            host + ":" + port,
            target
        );

        std::cout
            << "Connected to Binance depth stream\n";
    }

    void listen(
        trading::BoundedQueue<trading::DepthUpdate>& updates,
        std::atomic<bool>& overflowed) {

        beast::flat_buffer buffer;

        try {
            while (true) {
                buffer.clear();

                ws_.read(buffer);

                const std::string message =
                    beast::buffers_to_string(buffer.data());

                auto update =
                    trading::DepthParser::parse_update(message);

                if (!update) {
                    continue;
                }

                if (!updates.try_push(std::move(*update))) {
                    overflowed.store(true);

                    std::cerr
                        << "Depth update queue overflow!\n";
                }
            }
        }
        catch (const std::exception& e) {
            std::cerr
                << "WebSocket reader stopped: "
                << e.what() << '\n';
        }

        updates.close();

        std::cout
            << "WebSocket reader stopped\n";
    }

private:
    net::io_context ioc_;
    ssl::context ssl_context_;
    tcp::resolver resolver_;

    websocket::stream<
        beast::ssl_stream<beast::tcp_stream>
    > ws_;
};


int main() {
    constexpr std::size_t queue_capacity = 100'000;

    const CURLcode curl_result =
        curl_global_init(CURL_GLOBAL_DEFAULT);

    if (curl_result != CURLE_OK) {
        std::cerr << "Failed to initialize libcurl\n";
        return EXIT_FAILURE;
    }

    int exit_code = EXIT_SUCCESS;

    {
        trading::BoundedQueue<trading::DepthUpdate> updates(
            queue_capacity
        );

        std::atomic<bool> overflowed{false};

        trading::OrderBook order_book("BTCUSDT");

        BinanceMarketDataClient client;

        try {
            // Establish WebSocket connection.
            client.connect();

            // Start receiving updates in a separate thread.
            std::thread websocket_thread([&] {
                client.listen(updates, overflowed);
            });

            // Fetch snapshot and synchronize the order book.
            trading::DepthSynchronizer synchronizer(
                "BTCUSDT",
                updates,
                overflowed,
                order_book
            );

            synchronizer.run();

            // Wait for the WebSocket reader to finish.
            websocket_thread.join();
        }
        catch (const std::exception& e) {
            std::cerr
                << "Application error: "
                << e.what() << '\n';

            exit_code = EXIT_FAILURE;
        }
    }

    curl_global_cleanup();

    return exit_code;
}