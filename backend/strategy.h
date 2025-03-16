#pragma once

#include "market_feed.h"
#include "order_book.h"
#include <vector>
#include <map>
#include <deque>
#include <functional>

class MomentumStrategy
{
public:
    MomentumStrategy(OrderBook &order_book);
    ~MomentumStrategy();

    void onMarketTick(const MarketTick &tick);
    void setOrderCallback(std::function<void(const Order &)> callback);

    // Strategy parameters
    void setTickThreshold(int threshold);
    void setOrderQuantity(int quantity);

private:
    void checkMomentum(const std::string &venue);
    bool isUpwardMomentum(const std::string &venue) const;
    bool isDownwardMomentum(const std::string &venue) const;

    OrderBook &order_book_;
    std::function<void(const Order &)> order_callback_;

    // Price history for momentum calculation
    std::map<std::string, std::deque<double>> price_history_;

    // Strategy parameters
    int tick_threshold_;
    int order_quantity_;

    // Order tracking
    int order_counter_;

    static constexpr int MAX_PRICE_HISTORY = 10;
};