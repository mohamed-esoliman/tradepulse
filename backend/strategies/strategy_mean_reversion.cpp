#include "strategies/strategy_mean_reversion.h"
#include <chrono>

void MeanReversionStrategy::onMarketTick(const MarketTick &tick)
{
    auto &hist = price_history_[tick.venue];
    hist.push_back(tick.price);
    if (hist.size() > static_cast<size_t>(lookback_))
        hist.pop_front();
    if (hist.size() < static_cast<size_t>(lookback_))
        return;
    double sum = 0.0;
    for (double p : hist)
        sum += p;
    double avg = sum / hist.size();
    double last = hist.back();
    if (last < avg)
    {
        Order o;
        o.id = "O" + std::to_string(++order_counter_);
        o.venue = tick.venue;
        o.symbol = tick.symbol;
        o.side = OrderSide::BUY;
        o.price = last;
        o.quantity = order_quantity_;
        o.timestamp = std::chrono::system_clock::now();
        o.exchange_recv_ts_ms = tick.exchange_recv_ts_ms;
        o.ingest_ts_ms = tick.ingest_ts_ms;
        if (on_order)
            on_order(o);
    }
    else if (last > avg)
    {
        Order o;
        o.id = "O" + std::to_string(++order_counter_);
        o.venue = tick.venue;
        o.symbol = tick.symbol;
        o.side = OrderSide::SELL;
        o.price = last;
        o.quantity = order_quantity_;
        o.timestamp = std::chrono::system_clock::now();
        o.exchange_recv_ts_ms = tick.exchange_recv_ts_ms;
        o.ingest_ts_ms = tick.ingest_ts_ms;
        if (on_order)
            on_order(o);
    }
}
