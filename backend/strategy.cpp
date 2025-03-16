#include "strategy.h"
#include <iostream>
#include <sstream>

MomentumStrategy::MomentumStrategy(OrderBook &order_book)
    : order_book_(order_book), tick_threshold_(3), order_quantity_(100), order_counter_(0)
{
}

MomentumStrategy::~MomentumStrategy()
{
}

void MomentumStrategy::onMarketTick(const MarketTick &tick)
{
    // Store price history
    auto &history = price_history_[tick.venue];
    history.push_back(tick.price);

    // Maintain maximum history size
    if (history.size() > MAX_PRICE_HISTORY)
    {
        history.pop_front();
    }

    // Check for momentum signals
    if (history.size() >= tick_threshold_)
    {
        checkMomentum(tick.venue);
    }
}

void MomentumStrategy::setOrderCallback(std::function<void(const Order &)> callback)
{
    order_callback_ = callback;
}

void MomentumStrategy::setTickThreshold(int threshold)
{
    tick_threshold_ = threshold;
}

void MomentumStrategy::setOrderQuantity(int quantity)
{
    order_quantity_ = quantity;
}

void MomentumStrategy::checkMomentum(const std::string &venue)
{
    if (isUpwardMomentum(venue))
    {
        // Send BUY order
        Order order;
        order.id = "O" + std::to_string(++order_counter_);
        order.venue = venue;
        order.side = OrderSide::BUY;
        order.price = price_history_[venue].back();
        order.quantity = order_quantity_;
        order.timestamp = std::chrono::system_clock::now();

        // Submit order to order book
        order_book_.submitOrder(order);

        // Call order callback if set
        if (order_callback_)
        {
            order_callback_(order);
        }

        std::cout << "BUY signal for " << venue << " at $" << order.price << std::endl;
    }
    else if (isDownwardMomentum(venue))
    {
        // Send SELL order
        Order order;
        order.id = "O" + std::to_string(++order_counter_);
        order.venue = venue;
        order.side = OrderSide::SELL;
        order.price = price_history_[venue].back();
        order.quantity = order_quantity_;
        order.timestamp = std::chrono::system_clock::now();

        // Submit order to order book
        order_book_.submitOrder(order);

        // Call order callback if set
        if (order_callback_)
        {
            order_callback_(order);
        }

        std::cout << "SELL signal for " << venue << " at $" << order.price << std::endl;
    }
}

bool MomentumStrategy::isUpwardMomentum(const std::string &venue) const
{
    const auto &history = price_history_.at(venue);

    if (history.size() < tick_threshold_)
    {
        return false;
    }

    // Check if last N ticks are all increasing
    for (int i = history.size() - tick_threshold_; i < history.size() - 1; ++i)
    {
        if (history[i] >= history[i + 1])
        {
            return false;
        }
    }

    return true;
}

bool MomentumStrategy::isDownwardMomentum(const std::string &venue) const
{
    const auto &history = price_history_.at(venue);

    if (history.size() < tick_threshold_)
    {
        return false;
    }

    // Check if last N ticks are all decreasing
    for (int i = history.size() - tick_threshold_; i < history.size() - 1; ++i)
    {
        if (history[i] <= history[i + 1])
        {
            return false;
        }
    }

    return true;
}