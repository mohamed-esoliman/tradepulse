#include "strategy_rsi.h"
#include <chrono>

static double compute_rsi(const std::deque<double> &p, int period)
{
    if (p.size() < static_cast<size_t>(period + 1))
        return 50.0;
    double gains = 0.0, losses = 0.0;
    for (int i = p.size() - period; i < static_cast<int>(p.size()); ++i)
    {
        double diff = p[i] - p[i - 1];
        if (diff > 0)
            gains += diff;
        else
            losses -= diff;
    }
    double rs = losses == 0 ? 0 : gains / (losses == 0 ? 1 : losses);
    double rsi = 100.0 - (100.0 / (1.0 + rs));
    return rsi;
}

void RsiStrategy::onMarketTick(const MarketTick &tick)
{
    auto &px = prices_[tick.venue];
    px.push_back(tick.price);
    if (px.size() > static_cast<size_t>(period_ + 1))
        px.pop_front();
    if (px.size() < static_cast<size_t>(period_ + 1))
        return;

    double rsi = compute_rsi(px, period_);
    if (rsi < 30.0)
    {
        Order o;
        o.id = "O" + std::to_string(++order_counter_);
        o.venue = tick.venue;
        o.symbol = tick.symbol;
        o.side = OrderSide::BUY;
        o.price = tick.price;
        o.quantity = order_qty_;
        o.timestamp = std::chrono::system_clock::now();
        o.exchange_recv_ts_ms = tick.exchange_recv_ts_ms;
        o.ingest_ts_ms = tick.ingest_ts_ms;
        if (on_order)
            on_order(o);
    }
    else if (rsi > 70.0)
    {
        Order o;
        o.id = "O" + std::to_string(++order_counter_);
        o.venue = tick.venue;
        o.symbol = tick.symbol;
        o.side = OrderSide::SELL;
        o.price = tick.price;
        o.quantity = order_qty_;
        o.timestamp = std::chrono::system_clock::now();
        o.exchange_recv_ts_ms = tick.exchange_recv_ts_ms;
        o.ingest_ts_ms = tick.ingest_ts_ms;
        if (on_order)
            on_order(o);
    }
}
