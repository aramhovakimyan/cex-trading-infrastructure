#pragma once

#include "market_data/depth_event.hpp"

#include <cstdint>
#include <map>
#include <string>
#include <functional>
#include <cstddef>
#include <vector>

namespace trading {

enum class UpdateResult {
    Applied,
    Ignored,
    GapDetected,
    InvalidSymbol
};

class OrderBook {
public:
    explicit OrderBook(std::string symbol);

    void load_snapshot(const DepthSnapshot& snapshot);

    UpdateResult apply_update(
        const DepthUpdate& update
    );

    std::uint64_t last_update_id() const;

    Price best_bid() const;
    Price best_ask() const;

    Quantity best_bid_quantity() const;
    Quantity best_ask_quantity() const;

    std::size_t bid_levels() const;
    std::size_t ask_levels() const;

    bool synchronized() const;

private:
    void apply_levels(
        std::map<Price, Quantity, std::greater<>>& book,
        const std::vector<PriceLevel>& levels
    );

    void apply_levels(
        std::map<Price, Quantity>& book,
        const std::vector<PriceLevel>& levels
    );

    std::string symbol_;

    std::uint64_t last_update_id_{0};

    bool synchronized_{false};

    std::map<Price, Quantity, std::greater<>> bids_;
    std::map<Price, Quantity> asks_;
};

}

