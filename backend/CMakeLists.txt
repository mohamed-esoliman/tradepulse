cmake_minimum_required(VERSION 3.10)
project(TradePulse)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# CMake policy for Boost (suppress CMP0167 warning on newer CMake)
if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

# Find required packages
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)

# Check for OpenSSL
find_package(OpenSSL REQUIRED COMPONENTS Crypto SSL)
find_package(Boost REQUIRED COMPONENTS system)

# Add executable
add_executable(tradepulse
    main.cpp
    data_source.h
    config.h
    config.cpp
    market_feed.cpp
    order_book.cpp
    strategies/strategy_base.h
    strategies/strategy_momentum.h
    strategies/strategy_momentum.cpp
    strategies/strategy_mean_reversion.h
    strategies/strategy_mean_reversion.cpp
    strategies/strategy_breakout.h
    strategies/strategy_breakout.cpp
    strategies/strategy_vwap_reversion.h
    strategies/strategy_vwap_reversion.cpp
    strategies/strategy_macd.h
    strategies/strategy_macd.cpp
    strategies/strategy_rsi.h
    strategies/strategy_rsi.cpp
    strategies/strategy_bollinger.h
    strategies/strategy_bollinger.cpp
    latency.cpp
    websocket_server.cpp
    replay_feed.h
    replay_feed.cpp
    live_feed_coinbase.h
    live_feed_coinbase.cpp
)

# Link libraries
target_link_libraries(tradepulse 
    ${CMAKE_THREAD_LIBS_INIT}
    OpenSSL::SSL
    OpenSSL::Crypto
    Boost::system
)

# Include directories
target_include_directories(tradepulse PRIVATE .)

# Compiler flags
target_compile_options(tradepulse PRIVATE -Wall -Wextra -O2)

# Install target
install(TARGETS tradepulse DESTINATION bin) 