#pragma once

#include "market_data/depth_event.hpp"

#include <optional>
#include <string>

namespace trading {

class DepthParser {
public:
    static std::optional<DepthUpdate>
    parse_update(const std::string& message);

    static std::optional<DepthSnapshot>
    parse_snapshot(const std::string& message);
};

}

