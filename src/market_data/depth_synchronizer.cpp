#include "market_data/depth_synchronizer.hpp"
#include "market_data/depth_snapshot_client.hpp"

#include <chrono>
#include <iostream>
#include <thread>
#include <utility>

namespace trading {

DepthSynchronizer::DepthSynchronizer(
    std::string symbol,
    BoundedQueue<DepthUpdate>& updates,
    std::atomic<bool>& overflowed,
    OrderBook& book)
    : symbol_(std::move(symbol)),
      updates_(updates),
      overflowed_(overflowed),
      book_(book) {}

void DepthSynchronizer::run() {
    using namespace std::chrono_literals;

    while (true) {
        std::cout << "Fetching depth snapshot...\n";

        auto snapshot = DepthSnapshotClient::fetch(symbol_);

        if (!snapshot) {
            std::cerr << "Snapshot request failed. Retrying...\n";
            std::this_thread::sleep_for(2s);
            continue;
        }

        book_.load_snapshot(*snapshot);

        std::cout << "Snapshot loaded. lastUpdateId = "
                  << snapshot->last_update_id << '\n';

        bool resynchronize = false;
        bool first_update_applied = false;

        while (!resynchronize) {
            // An overflow means some market updates were lost.
            if (overflowed_.exchange(false)) {
                std::cerr << "Update queue overflow. Resynchronizing...\n";
                resynchronize = true;
                break;
            }

            auto update = updates_.wait_pop();

            if (!update) {
                std::cerr << "Update queue closed. "
                          << "Stopping synchronizer.\n";
                return;
            }

            if (update->symbol != symbol_) {
                continue;
            }

            const auto current_id = book_.last_update_id();

            // This update is already covered by our snapshot/book.
            if (update->final_update_id <= current_id) {
                continue;
            }

            // The next expected update is missing.
            if (update->first_update_id > current_id + 1) {
                std::cerr << "Sequence gap detected. Resynchronizing...\n";
                resynchronize = true;
                break;
            }

            // The update must bridge current_id + 1.
            if (update->final_update_id < current_id + 1) {
                continue;
            }

            const auto result = book_.apply_update(*update);

            if (result == UpdateResult::GapDetected) {
                std::cerr << "Order book rejected update due to a gap.\n";
                resynchronize = true;
                break;
            }

            if (result == UpdateResult::Applied && !first_update_applied) {
                first_update_applied = true;
                std::cout << "Order book synchronized. Update ID = "
                          << book_.last_update_id() << '\n';
            }

            if (result == UpdateResult::Applied) {
                // Temporary observability; replace with metrics later.
                // Print only occasionally in the next iteration.
            }
        }
    }
}

} // namespace trading
