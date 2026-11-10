#pragma once

#include "market_data/depth_event.hpp"

#include <optional>
#include <string>

namespace trading {

class DepthSnapshotClient {
public:
    static std::optional<DepthSnapshot> fetch(
        const std::string& symbol);
};

} // namespace trading
