#pragma once

#include <string>
#include <cstdint>
#include <functional>

struct MarketTick
{
    std::string venue;
    std::string symbol;
    double price;
    double size;
    int64_t exchange_recv_ts_ms;
    int64_t ingest_ts_ms;
};

class IDataSource
{
public:
    virtual ~IDataSource() = default;
    virtual void start(std::function<void(const MarketTick &)> on_tick) = 0;
    virtual void stop() = 0;
};
