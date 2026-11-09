#include "market_data/order_book.hpp"

#include <stdexcept>

namespace trading {

void OrderBook::update(const MarketDataEvent& event) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (snapshot_ &&
        event.symbol == snapshot_->symbol &&
        event.update_id != 0 &&
        snapshot_->update_id != 0 &&
        event.update_id < snapshot_->update_id) {
        return;
    }

    snapshot_ = event;
}

std::optional<MarketDataEvent>
OrderBook::get_snapshot() const {
    std::lock_guard<std::mutex> lock(mutex_);

    return snapshot_;
}

double OrderBook::spread() const {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!snapshot_) {
        return 0.0;
    }

    return snapshot_->ask_price -
           snapshot_->bid_price;
}

double OrderBook::midpoint() const {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!snapshot_) {
        return 0.0;
    }

    return (
        snapshot_->ask_price +
        snapshot_->bid_price
    ) / 2.0;
}

}

