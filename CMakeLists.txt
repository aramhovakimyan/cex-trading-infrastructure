cmake_minimum_required(VERSION 3.20)

project(CEXTradingInfrastructure
        VERSION 0.2.0
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost REQUIRED COMPONENTS system)
find_package(OpenSSL REQUIRED)
find_package(nlohmann_json REQUIRED)

add_executable(market_data_client
        src/market_data/main.cpp
        src/market_data/market_data_parser.cpp
        src/market_data/order_book.cpp
)

target_include_directories(market_data_client
        PRIVATE include
)

target_link_libraries(market_data_client
        PRIVATE
        Boost::system
        OpenSSL::SSL
        OpenSSL::Crypto
        nlohmann_json::nlohmann_json
)

target_compile_options(market_data_client
        PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -O2
)
