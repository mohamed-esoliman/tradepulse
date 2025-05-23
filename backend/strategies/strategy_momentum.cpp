#include "strategies/strategy_momentum.h"
#include <chrono>
#include <iostream>

void MomentumStrategy::onMarketTick(const MarketTick &tick)
{
    auto &history = price_history_[tick.venue];
    history.push_back(tick.price);
    if (history.size() > MAX_PRICE_HISTORY)
        history.pop_front();
    if (history.size() >= static_cast<size_t>(tick_threshold_))
    {
        checkMomentum(tick.venue, tick);
    }
}

void MomentumStrategy::checkMomentum(const std::string &venue, const MarketTick &tick)
{
    if (isUpwardMomentum(venue))
    {
        Order order;
        order.id = "O" + std::to_string(++order_counter_);
        order.venue = venue;
        order.symbol = tick.symbol;
        order.side = OrderSide::BUY;
        order.price = tick.price;
        order.quantity = order_quantity_;
        order.timestamp = std::chrono::system_clock::now();
        order.exchange_recv_ts_ms = tick.exchange_recv_ts_ms;
        order.ingest_ts_ms = tick.ingest_ts_ms;
        if (on_order)
            on_order(order);
    }
    else if (isDownwardMomentum(venue))
    {
        Order order;
        order.id = "O" + std::to_string(++order_counter_);
        order.venue = venue;
        order.symbol = tick.symbol;
        order.side = OrderSide::SELL;
        order.price = tick.price;
        order.quantity = order_quantity_;
        order.timestamp = std::chrono::system_clock::now();
        order.exchange_recv_ts_ms = tick.exchange_recv_ts_ms;
        order.ingest_ts_ms = tick.ingest_ts_ms;
        if (on_order)
            on_order(order);
    }
}

bool MomentumStrategy::isUpwardMomentum(const std::string &venue) const
{
    const auto &h = price_history_.at(venue);
    if (h.size() < static_cast<size_t>(tick_threshold_))
        return false;
    for (size_t i = h.size() - tick_threshold_; i < h.size() - 1; ++i)
        if (h[i] >= h[i + 1])
            return false;
    return true;
}

bool MomentumStrategy::isDownwardMomentum(const std::string &venue) const
{
    const auto &h = price_history_.at(venue);
    if (h.size() < static_cast<size_t>(tick_threshold_))
        return false;
    for (size_t i = h.size() - tick_threshold_; i < h.size() - 1; ++i)
        if (h[i] <= h[i + 1])
            return false;
    return true;
}
