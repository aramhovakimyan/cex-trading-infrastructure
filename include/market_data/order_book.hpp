#pragma once

#include "market_data/market_data_event.hpp"

#include <mutex>
#include <optional>
#include <string>

namespace trading {

class OrderBook {
public:
    void update(const MarketDataEvent& event);

    std::optional<MarketDataEvent>
    get_snapshot() const;

    double spread() const;

    double midpoint() const;

private:
    mutable std::mutex mutex_;

    std::optional<MarketDataEvent> snapshot_;
};

}

