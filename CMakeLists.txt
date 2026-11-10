cmake_minimum_required(VERSION 3.20)

project(CEXTradingInfrastructure
        VERSION 0.3.0
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost REQUIRED COMPONENTS system)
find_package(OpenSSL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(CURL REQUIRED)

add_library(market_data_core
        src/market_data/main.cpp
        src/market_data/market_data_parser.cpp
        src/market_data/order_book.cpp
        src/market_data/depth_parser.cpp
        src/market_data/depth_snapshot_client.cpp
        src/market_data/depth_synchronizer.cpp
)

target_include_directories(market_data_core
        PUBLIC include
)

target_link_libraries(market_data_core
        PUBLIC
        nlohmann_json::nlohmann_json
        CURL::libcurl
)

target_compile_options(market_data_core
        PRIVATE -Wall -Wextra -Wpedantic
)

add_executable(order_book_test
        tests/order_book_test.cpp
)

target_link_libraries(order_book_test
        PRIVATE market_data_core
)

enable_testing()

add_test(
        NAME OrderBookTest
        COMMAND order_book_test
)

add_executable(bounded_queue_test
        tests/bounded_queue_test.cpp
)

target_link_libraries(bounded_queue_test
        PRIVATE market_data_core
)

add_test(
        NAME BoundedQueueTest
        COMMAND bounded_queue_test
)

# Keep the existing WebSocket client target.
# Link market_data_core to it when integrating depth processing.
