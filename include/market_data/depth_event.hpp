#pragma once

#include <cstdint>
#include <vector>
#include <string>

namespace trading {

using Price = std::int64_t;
using Quantity = std::int64_t;

inline constexpr std::int64_t SCALE = 100'000'000;

struct PriceLevel {
    Price price{};
    Quantity quantity{};
};

struct DepthUpdate {
    std::string symbol;

    std::uint64_t first_update_id{};
    std::uint64_t final_update_id{};

    std::vector<PriceLevel> bids;
    std::vector<PriceLevel> asks;
};

struct DepthSnapshot {
    std::string symbol;

    std::uint64_t last_update_id{};

    std::vector<PriceLevel> bids;
    std::vector<PriceLevel> asks;
};

}

