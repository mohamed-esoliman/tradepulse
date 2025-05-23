#include "strategy_vwap_reversion.h"
#include <chrono>

void VwapReversionStrategy::onMarketTick(const MarketTick &tick)
{
    auto &dq = window_[tick.venue];
    dq.emplace_back(tick.price, tick.size > 0 ? tick.size : 1.0);
    if (dq.size() > static_cast<size_t>(lookback_))
        dq.pop_front();
    if (dq.size() < 2)
        return;

    double sum_pv = 0.0, sum_v = 0.0;
    for (auto &ps : dq)
    {
        sum_pv += ps.first * ps.second;
        sum_v += ps.second;
    }
    double vwap = sum_pv / (sum_v > 0 ? sum_v : 1.0);
    double last = dq.back().first;

    if (last < vwap)
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
    else if (last > vwap)
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
