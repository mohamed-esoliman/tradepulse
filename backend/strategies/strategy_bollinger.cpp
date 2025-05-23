#include "strategy_bollinger.h"
#include <chrono>
#include <cmath>

void BollingerStrategy::onMarketTick(const MarketTick &tick)
{
    auto &px = prices_[tick.venue];
    px.push_back(tick.price);
    if (px.size() > static_cast<size_t>(period_))
        px.pop_front();
    if (px.size() < static_cast<size_t>(period_))
        return;

    double sum = 0.0;
    for (double p : px)
        sum += p;
    double mean = sum / px.size();
    double var = 0.0;
    for (double p : px)
        var += (p - mean) * (p - mean);
    var /= px.size();
    double sd = std::sqrt(var);
    double upper = mean + k_ * sd;
    double lower = mean - k_ * sd;
    double last = px.back();

    if (last < lower)
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
    else if (last > upper)
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
