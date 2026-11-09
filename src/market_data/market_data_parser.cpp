#include "market_data/market_data_parser.hpp"

#include <nlohmann/json.hpp>

#include <stdexcept>

namespace trading {

using json = nlohmann::json;

std::optional<MarketDataEvent>
MarketDataParser::parse_book_ticker(
    const std::string& message
) {
    try {
        const auto data = json::parse(message);

        MarketDataEvent event;

        event.symbol =
            data.at("s").get<std::string>();

        event.bid_price =
            std::stod(data.at("b").get<std::string>());

        event.bid_quantity =
            std::stod(data.at("B").get<std::string>());

        event.ask_price =
            std::stod(data.at("a").get<std::string>());

        event.ask_quantity =
            std::stod(data.at("A").get<std::string>());

        if (data.contains("u")) {
            event.update_id =
                data.at("u").get<std::uint64_t>();
        }

        if (event.bid_price <= 0 ||
            event.ask_price <= 0 ||
            event.bid_quantity < 0 ||
            event.ask_quantity < 0 ||
            event.bid_price > event.ask_price) {
            return std::nullopt;
        }

        event.received_at =
            std::chrono::steady_clock::now();

        return event;

    } catch (const std::exception&) {
        return std::nullopt;
    }
}

}

