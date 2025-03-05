cmake_minimum_required(VERSION 3.10)
project(TradePulse)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)

# Check for OpenSSL
find_package(OpenSSL REQUIRED)

# Add executable
add_executable(tradepulse
    main.cpp
    market_feed.cpp
    order_book.cpp
    strategy.cpp
    latency.cpp
    websocket_server.cpp
)

# Link libraries
target_link_libraries(tradepulse 
    ${CMAKE_THREAD_LIBS_INIT}
    OpenSSL::SSL
    OpenSSL::Crypto
)

# Include directories
target_include_directories(tradepulse PRIVATE .)

# Compiler flags
target_compile_options(tradepulse PRIVATE -Wall -Wextra -O2)

# Install target
install(TARGETS tradepulse DESTINATION bin) 