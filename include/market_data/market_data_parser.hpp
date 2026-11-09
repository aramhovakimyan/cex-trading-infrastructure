#pragma once

#include "market_data/market_data_event.hpp"

#include <optional>
#include <string>

namespace trading {

class MarketDataParser {
public:
    static std::optional<MarketDataEvent>
    parse_book_ticker(const std::string& message);
};

}

