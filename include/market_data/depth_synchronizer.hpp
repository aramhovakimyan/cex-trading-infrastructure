#pragma once

#include "market_data/bounded_queue.hpp"
#include "market_data/depth_event.hpp"
#include "market_data/order_book.hpp"

#include <atomic>
#include <string>

namespace trading {

class DepthSynchronizer {
public:
    DepthSynchronizer(
        std::string symbol,
        BoundedQueue<DepthUpdate>& updates,
        std::atomic<bool>& overflowed,
        OrderBook& book);

    void run();

private:
    std::string symbol_;
    BoundedQueue<DepthUpdate>& updates_;
    std::atomic<bool>& overflowed_;
    OrderBook& book_;
};

} // namespace trading
