#pragma once

#include <chrono>
#include <cstdint>
#include <string>

namespace trading {

struct MarketDataEvent {
    std::string symbol;

    double bid_price{0.0};
    double bid_quantity{0.0};

    double ask_price{0.0};
    double ask_quantity{0.0};

    std::uint64_t update_id{0};

    std::chrono::steady_clock::time_point
        received_at;
};

}

