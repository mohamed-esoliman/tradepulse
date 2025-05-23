#include "strategy_breakout.h"
#include <chrono>

void BreakoutStrategy::onMarketTick(const MarketTick &tick)
{
    auto &dq = window_[tick.venue];
    dq.push_back(tick.price);
    if (dq.size() > static_cast<size_t>(lookback_))
        dq.pop_front();
    if (dq.size() < static_cast<size_t>(lookback_))
        return;

    double highest = dq.front();
    double lowest = dq.front();
    for (double p : dq)
    {
        if (p > highest)
            highest = p;
        if (p < lowest)
            lowest = p;
    }
    double last = dq.back();

    if (last > highest)
    {
        Order o;
        o.id = "O" + std::to_string(++order_counter_);
        o.venue = tick.venue;
        o.symbol = tick.symbol;
        o.side = OrderSide::BUY;
        o.price = last;
        o.quantity = order_qty_;
        o.timestamp = std::chrono::system_clock::now();
        o.exchange_recv_ts_ms = tick.exchange_recv_ts_ms;
        o.ingest_ts_ms = tick.ingest_ts_ms;
        if (on_order)
            on_order(o);
    }
    else if (last < lowest)
    {
        Order o;
        o.id = "O" + std::to_string(++order_counter_);
        o.venue = tick.venue;
        o.symbol = tick.symbol;
        o.side = OrderSide::SELL;
        o.price = last;
        o.quantity = order_qty_;
        o.timestamp = std::chrono::system_clock::now();
        o.exchange_recv_ts_ms = tick.exchange_recv_ts_ms;
        o.ingest_ts_ms = tick.ingest_ts_ms;
        if (on_order)
            on_order(o);
    }
}
