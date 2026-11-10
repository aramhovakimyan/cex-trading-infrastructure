
#include "market_data/order_book.hpp"

#include <stdexcept>
#include <utility>

namespace trading {

OrderBook::OrderBook(std::string symbol)
    : symbol_(std::move(symbol)) {}

void OrderBook::load_snapshot(
    const DepthSnapshot& snapshot
) {
    if (snapshot.symbol != symbol_) {
        throw std::invalid_argument("Snapshot symbol mismatch");
    }

    bids_.clear();
    asks_.clear();

    apply_levels(bids_, snapshot.bids);
    apply_levels(asks_, snapshot.asks);

    last_update_id_ = snapshot.last_update_id;
    synchronized_ = true;
}

UpdateResult OrderBook::apply_update(
    const DepthUpdate& update
) {
    if (update.symbol != symbol_) {
        return UpdateResult::InvalidSymbol;
    }

    if (!synchronized_) {
        return UpdateResult::Ignored;
    }

    // Ignore updates already covered by the book.
    if (update.final_update_id <= last_update_id_) {
        return UpdateResult::Ignored;
    }

    // A missing update means our local book is unreliable.
    if (update.first_update_id > last_update_id_ + 1) {
        synchronized_ = false;
        return UpdateResult::GapDetected;
    }

    apply_levels(bids_, update.bids);
    apply_levels(asks_, update.asks);

    last_update_id_ = update.final_update_id;

    return UpdateResult::Applied;
}

void OrderBook::apply_levels(
    std::map<Price, Quantity, std::greater<>>& book,
    const std::vector<PriceLevel>& levels
) {
    for (const auto& level : levels) {
        if (level.quantity == 0) {
            book.erase(level.price);
        } else {
            book[level.price] = level.quantity;
        }
    }
}

void OrderBook::apply_levels(
    std::map<Price, Quantity>& book,
    const std::vector<PriceLevel>& levels
) {
    for (const auto& level : levels) {
        if (level.quantity == 0) {
            book.erase(level.price);
        } else {
            book[level.price] = level.quantity;
        }
    }
}

std::uint64_t OrderBook::last_update_id() const {
    return last_update_id_;
}

Price OrderBook::best_bid() const {
    return bids_.empty() ? 0 : bids_.begin()->first;
}

Price OrderBook::best_ask() const {
    return asks_.empty() ? 0 : asks_.begin()->first;
}

Quantity OrderBook::best_bid_quantity() const {
    return bids_.empty() ? 0 : bids_.begin()->second;
}

Quantity OrderBook::best_ask_quantity() const {
    return asks_.empty() ? 0 : asks_.begin()->second;
}

std::size_t OrderBook::bid_levels() const {
    return bids_.size();
}

std::size_t OrderBook::ask_levels() const {
    return asks_.size();
}

bool OrderBook::synchronized() const {
    return synchronized_;
}

}

