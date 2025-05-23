#include "strategy_mean_reversion.h"
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
    double avg = sum / static_cast<double>(hist.size());
    double last = hist.back();

    if (last < avg)
    {
        Order order;
        order.id = "O" + std::to_string(++order_counter_);
        order.venue = tick.venue;
        order.symbol = tick.symbol;
        order.side = OrderSide::BUY;
        order.price = last;
        order.quantity = order_quantity_;
        order.timestamp = std::chrono::system_clock::now();
        order.exchange_recv_ts_ms = tick.exchange_recv_ts_ms;
        order.ingest_ts_ms = tick.ingest_ts_ms;
        if (on_order)
            on_order(order);
    }
    else if (last > avg)
    {
        Order order;
        order.id = "O" + std::to_string(++order_counter_);
        order.venue = tick.venue;
        order.symbol = tick.symbol;
        order.side = OrderSide::SELL;
        order.price = last;
        order.quantity = order_quantity_;
        order.timestamp = std::chrono::system_clock::now();
        order.exchange_recv_ts_ms = tick.exchange_recv_ts_ms;
        order.ingest_ts_ms = tick.ingest_ts_ms;
        if (on_order)
            on_order(order);
    }
}
