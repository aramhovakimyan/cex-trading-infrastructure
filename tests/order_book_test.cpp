#include "market_data/order_book.hpp"

#include <cassert>
#include <iostream>

using namespace trading;

int main() {
    OrderBook book("BTCUSDT");

    // Initial snapshot
    DepthSnapshot snapshot;

    snapshot.symbol = "BTCUSDT";
    snapshot.last_update_id = 100;

    snapshot.bids = {
        {67000 * SCALE, 100 * SCALE},
        {66999 * SCALE, 200 * SCALE}
    };

    snapshot.asks = {
        {67001 * SCALE, 150 * SCALE},
        {67002 * SCALE, 250 * SCALE}
    };

    book.load_snapshot(snapshot);

    assert(book.synchronized());
    assert(book.last_update_id() == 100);

    assert(book.best_bid() == 67000 * SCALE);
    assert(book.best_ask() == 67001 * SCALE);

    // Incremental update
    DepthUpdate update;

    update.symbol = "BTCUSDT";
    update.first_update_id = 101;
    update.final_update_id = 102;

    update.bids = {
        {67000 * SCALE, 0}
    };

    update.asks = {
        {67001 * SCALE, 200 * SCALE}
    };

    auto result = book.apply_update(update);

    assert(result == UpdateResult::Applied);

    assert(book.last_update_id() == 102);
    assert(book.best_bid() == 66999 * SCALE);
    assert(book.best_ask_quantity() == 200 * SCALE);

    // Stale event
    DepthUpdate stale;

    stale.symbol = "BTCUSDT";
    stale.first_update_id = 100;
    stale.final_update_id = 101;

    result = book.apply_update(stale);

    assert(result == UpdateResult::Ignored);

    // Simulate a missing update
    DepthUpdate gap;

    gap.symbol = "BTCUSDT";
    gap.first_update_id = 104;
    gap.final_update_id = 105;

    result = book.apply_update(gap);

    assert(result == UpdateResult::GapDetected);
    assert(!book.synchronized());

    std::cout << "All OrderBook tests passed!\n";

    return 0;
}

