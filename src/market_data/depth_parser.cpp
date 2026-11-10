#include "market_data/depth_parser.hpp"

#include <nlohmann/json.hpp>

#include <limits>
#include <stdexcept>

namespace trading {

using json = nlohmann::json;

namespace {

std::int64_t parse_decimal(const std::string& value) {
    if (value.empty()) {
        throw std::invalid_argument("Empty decimal");
    }

    std::size_t pos = 0;

    if (value[pos] == '-') {
        throw std::invalid_argument("Negative value");
    }

    std::int64_t whole = 0;
    std::int64_t fraction = 0;
    int fractional_digits = 0;

    while (pos < value.size() && value[pos] != '.') {
        const char c = value[pos++];

        if (c < '0' || c > '9') {
            throw std::invalid_argument("Invalid decimal");
        }

        const int digit = c - '0';

        if (whole >
            (std::numeric_limits<std::int64_t>::max()
             / SCALE - digit) / 10) {
            throw std::out_of_range("Decimal overflow");
        }

        whole = whole * 10 + digit;
    }

    if (pos < value.size() && value[pos] == '.') {
        ++pos;

        while (pos < value.size()) {
            const char c = value[pos++];

            if (c < '0' || c > '9' ||
                fractional_digits >= 8) {
                throw std::invalid_argument(
                    "Invalid decimal precision"
                );
            }

            fraction = fraction * 10 + (c - '0');
            ++fractional_digits;
        }
    }

    while (fractional_digits < 8) {
        fraction *= 10;
        ++fractional_digits;
    }

    if (whole >
        (std::numeric_limits<std::int64_t>::max()
         - fraction) / SCALE) {
        throw std::out_of_range("Decimal overflow");
    }

    return whole * SCALE + fraction;
}

std::vector<PriceLevel> parse_levels(
    const json& levels
) {
    if (!levels.is_array()) {
        throw std::invalid_argument("Expected levels array");
    }

    std::vector<PriceLevel> result;

    result.reserve(levels.size());

    for (const auto& level : levels) {
        if (!level.is_array() || level.size() != 2) {
            throw std::invalid_argument("Invalid price level");
        }

        Price price =
            parse_decimal(level[0].get<std::string>());

        Quantity quantity =
            parse_decimal(level[1].get<std::string>());

        if (price <= 0 || quantity < 0) {
            throw std::invalid_argument("Invalid level values");
        }

        result.push_back({price, quantity});
    }

    return result;
}

}

std::optional<DepthUpdate>
DepthParser::parse_update(const std::string& message) {
    try {
        const auto data = json::parse(message);

        if (data.at("e") != "depthUpdate") {
            return std::nullopt;
        }

        DepthUpdate event;

        event.symbol =
            data.at("s").get<std::string>();

        event.first_update_id =
            data.at("U").get<std::uint64_t>();

        event.final_update_id =
            data.at("u").get<std::uint64_t>();

        if (event.first_update_id >
            event.final_update_id) {
            return std::nullopt;
        }

        event.bids = parse_levels(data.at("b"));
        event.asks = parse_levels(data.at("a"));

        return event;

    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::optional<DepthSnapshot>
DepthParser::parse_snapshot(const std::string& message) {
    try {
        const auto data = json::parse(message);

        DepthSnapshot snapshot;

        snapshot.symbol = "BTCUSDT";

        snapshot.last_update_id =
            data.at("lastUpdateId").get<std::uint64_t>();

        snapshot.bids = parse_levels(data.at("bids"));
        snapshot.asks = parse_levels(data.at("asks"));

        return snapshot;

    } catch (const std::exception&) {
        return std::nullopt;
    }
}

}

